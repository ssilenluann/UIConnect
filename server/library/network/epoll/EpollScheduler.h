#ifndef NETWORK_EPOLL_EPOLLSCHEDULER_H
#define NETWORK_EPOLL_EPOLLSCHEDULER_H

#include "Epoll.h"
#include "EpollChannel.h"
#include "../../coroutine/Scheduler.h"
#include "../../TimerFunc.h"

class EpollScheduler: public Scheduler<Thread>
{
public:
    typedef std::shared_ptr<EpollScheduler> ptr;
    typedef RWMutex RWMutexType;

    EpollScheduler(size_t threadCount = 1, const std::string& name = "", bool callerThreadJoinWorker = false);
    ~EpollScheduler();
    
    static std::shared_ptr<EpollScheduler> GetThis();
    
    void start(std::shared_ptr<EpollScheduler> manager);

    TimerFunc::ptr addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot = true);
    TimerFunc::ptr addConditionTimer(
        uint64_t cycle, std::function<void()> cb,
        std::weak_ptr<void> weak_cond, bool oneshot = true
    );
    
    bool updateChannel(int action, int fd, std::shared_ptr<EpollChannel> channel, int event);

    enum EventType
    {
        NONE = 0, READ, WRITE
    };
protected:
    void notice() override;

private:
    int m_noticeFds[2];
    RWMutexType m_rwMutex;

};

#endif