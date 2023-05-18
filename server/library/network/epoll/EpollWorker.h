#ifndef NETWORK_EPOLL_EPOLLWORKER_H
#define NETWORK_EPOLL_EPOLLWORKER_H

#include "../../thread/Thread.h"
#include "../../TimerFunc.h"
#include "../../coroutine/Scheduler.h"
#include "Epoll.h"
#include "../../Mutex.h"
#include "EpollChannel.h"
#include <map>
class TcpSession;
class EpollWorker: public SchedulerWorker
{
public:
    typedef std::shared_ptr<EpollWorker> ptr;
    typedef RWMutex RWMutexType;

    EpollWorker(const std::string &name = "");
    void setScheduler(std::shared_ptr<Scheduler<EpollWorker>> sc);

    virtual void work() override;
    virtual void entry() override;
    virtual bool readyToQuit() override;

    virtual void bind(std::function<void(std::shared_ptr<EpollWorker>)> funcWithThread);

    TimerFunc::ptr addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot = true);
    TimerFunc::ptr addConditionTimer(
        uint64_t cycle, std::function<void()> cb,
        std::weak_ptr<void> weak_cond, bool oneshot = true
    );

    bool updateChannel(int action, int fd, std::shared_ptr<EpollChannel> channel, uint32_t event);
    
    void addSession(std::shared_ptr<TcpSession> session);
    std::shared_ptr<TcpSession> getSession(const unsigned long& sessionId);
    void removeSession(unsigned long sessionId);
    
private:
    std::weak_ptr<Scheduler<EpollWorker>> m_scheduler;
    std::function<void(std::shared_ptr<EpollWorker>)> m_schedulerFunc;

    std::atomic<size_t> m_pendingEventCount;
    std::map<unsigned long, std::shared_ptr<TcpSession>> m_sessions;
    std::vector<EpollChannel::ptr> m_activeChannels;

    Epoll::ptr m_epoller;
    TimerManager::ptr m_timer;
    RWMutexType m_rwMutex;
};
#endif