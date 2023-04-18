#ifndef TIMERFUNC_H
#define TIMERFUNC_H
#include <set>
#include "Mutex.h"
class TimerManager;

class TimerFunc: public std::enable_shared_from_this<TimerFunc>
{
    friend class TimerManager;
public:
    typedef std::shared_ptr<TimerFunc> ptr;

private:
    TimerFunc(uint64_t ms, std::function<void()> cb, bool oneshot, std::weak_ptr<TimerManager> manager);

public:
    bool cancel();

    // refresh timer func next called time
    bool refresh();

    /**
     * @brief reset the cycle time
     * @param[in]  ms, time diff
     * @param[in]  from_now, calculate the next call time from now
     * */ 
    bool reset(uint64_t ms, bool from_now);

private:

    bool m_oneshot = true;
    uint64_t m_cycle = 0;
    uint64_t m_nextCallTime = 0;
    std::function<void()> m_cb;
    std::weak_ptr<TimerManager> m_manager;

private:
    class TimeConparator
    {
    public:
        bool operator()(const TimerFunc::ptr& lhs, const TimerFunc::ptr& rhs) const;
    };
};

class TimerManager: public std::enable_shared_from_this<TimerManager>
{
friend class TimerFunc;
public:
    typedef RWMutex RWMutexType;

    TimerManager();
    virtual ~TimerManager();
    TimerFunc::ptr addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot = true);
    TimerFunc::ptr addConditionTimer(
        uint64_t cycle, std::function<void()> cb,
        std::weak_ptr<void> weak_cond, bool oneshot = false
    );

    uint64_t getNextTimer();
    void listExpiredCb(std::vector<std::function<void()>>& cbs);
    bool hasTimer();

    static void OnConditionTimer(std::weak_ptr<void> cond, std::function<void()> cb);
    
    // timer inserted into the front of m_timers
    void setOnTimerInsertedAtFront(std::function<void()> cb);
protected:

private:
    void queue(TimerFunc::ptr timer);

private:
    bool m_tickled = false;
    int64_t m_previousTime = 0;
    RWMutexType m_mutex;
    std::function<void()> m_onFrontInsertedCallback;
    std::set<TimerFunc::ptr, TimerFunc::TimeConparator> m_timers;
};
#endif