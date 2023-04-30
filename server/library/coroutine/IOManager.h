#ifndef COROUTINE_IOMANAGER_H
#define COROUTINE_IOMANAGER_H

#include "Scheduler.h"
#include "Coroutine.h"
#include "../TimerFunc.h"

struct FdContext;
class IOManager: public Scheduler
{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;

    IOManager(size_t threadCount = 1, const std::string& name = "", bool callerThreadJoinWorker = false);
    ~IOManager();
    
    enum EventType 
    {
        NONE = 0x0,
        READ = 0x1,     // EPOLLIN
        WRITE = 0x4,    // EPOLLOUT
    };


    bool addEvent(int fd, EventType event, std::function<void()> cb = nullptr);
    // do not trigger the callback func
    bool delEvent(int fd, EventType event);
    // if callback func exists, call it
    bool triggerAndCancelEvent(int fd, EventType event);
    bool triggerAndCancelEvent(int fd);
    static std::shared_ptr<IOManager> GetThis();
    
    void start(std::shared_ptr<IOManager> manager);

    TimerFunc::ptr addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot = true);
    TimerFunc::ptr addConditionTimer(
        uint64_t cycle, std::function<void()> cb,
        std::weak_ptr<void> weak_cond, bool oneshot = true
    );
    
protected:
    void notice() override;
    bool WorkCoroutineReadyToStop() override;
    void idle() override;

    bool WorkCoroutineReadyToStop(uint64_t& timeout);
    void contextResize(size_t size);

private:
    int m_epfd = 0;
    int m_noticeFds[2];
    std::atomic<size_t> m_pendingEventCount = {0};
    RWMutexType m_rwMutex;
    std::shared_ptr<TimerManager> m_timer;
    std::vector<std::shared_ptr<FdContext>> m_fdContexts;
};

// socket event context
struct FdContext
{
    typedef Mutex MutexType;

    // event context
    struct EventContext
    {
        std::weak_ptr<Scheduler> scheduler; // scheduer
        Coroutine::ptr coroutine;   // coroutine of event
        std::function<void()> cb;
    };

    EventContext& getContext(IOManager::EventType event);
    void resetContext(EventContext& ctx);
    void triggerEvent(IOManager::EventType event);
    
    int fd = 0;
    IOManager::EventType events = IOManager::EventType::NONE;     // current event type
    MutexType mutex;
    EventContext read;
    EventContext write;
};
#endif