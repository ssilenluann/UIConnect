#ifndef NETWORK_EPOLL_EPOLLWORKER_CPP
#define NETWORK_EPOLL_EPOLLWORKER_CPP

#include "EpollWorker.h"
#include "../TcpSession.h"
#include "../../log/Logger.h"
#include <memory> 

static Logger::ptr g_logger = LOG_NAME("system");

EpollWorker::EpollWorker(const std::string & name)
    :m_pendingEventCount(0), m_epoller(new Epoll()), m_timer(new TimerManager()),
    SchedulerWorker(name) 
{
    LOG_ASSERT(m_epoller->fd() > 0);
}

void EpollWorker::setScheduler(std::shared_ptr<Scheduler<EpollWorker>> sc)
{
    m_scheduler = sc;
}

void EpollWorker::work()
{
    LOG_DEBUG(g_logger) << "idle";
    const uint64_t MAX_EVENT_COUNT = 256;

    while(!readyToQuit())
    {
        uint64_t next_timeout = 0;
        int cntAct = 0;
        do
        {
            static const int MAX_TIMEOUT = 3000;
            next_timeout = (next_timeout > MAX_TIMEOUT || next_timeout == 0)
                ? MAX_TIMEOUT : next_timeout;

            cntAct = m_epoller->poll(m_activeChannels, next_timeout);
            // cntAct = epoll_wait(m_epfd, events, MAX_EVENT_COUNT, (int)next_timeout);
            if(cntAct < 0 && errno == EINTR)
                continue;   // interrupted, conitnue
            if(cntAct < 0)
                LOG_DEBUG(g_logger) << "epoll error, instace fd = " << m_epoller->fd()
                    << ", errno = " << errno << ", err info: " << strerror(errno);
            
            // whatever notified or timeout, break
            break;
        }while(true);

        for(int i = 0; i < cntAct; ++i)
        {

            // process events
            RWMutexType::ReadLock lock(m_rwMutex);
            if(m_sessions.find(m_activeChannels[i]->fd()) == m_sessions.end())
            {
                LOG_ERROR(g_logger) << "unknown fd event";
                continue;
            }
            
            m_activeChannels[i]->handleEvent();
            lock.unlock();
        }

        // call timer
        std::vector<std::function<void()>> cbs;
        m_timer->listExpiredCb(cbs);
        // LOG_INFO(g_logger) << "expired timer count = " << cbs.size();
        for(auto& cb: cbs)
            m_scheduler.lock()->schedule(cb);

        cbs.clear();

        // timer func and epoll event processe finish, swapout
        auto curCoroutine = Coroutine::GetThreadCurrCoroutine();
        auto raw_ptr = curCoroutine.get();
        curCoroutine.reset();

        raw_ptr->swapOut();
    }
}

void EpollWorker::bind(std::function<void(std::shared_ptr<EpollWorker>)> funcWithThread)
{
    m_schedulerFunc = funcWithThread;
}

TimerFunc::ptr EpollWorker::addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot)
{
    return m_timer->addTimer(cycle, cb, oneshot);
}

TimerFunc::ptr EpollWorker::addConditionTimer(uint64_t cycle, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool oneshot)
{
    return m_timer->addConditionTimer(cycle, cb, weak_cond, oneshot);
}

bool EpollWorker::updateChannel(int action, int fd, std::shared_ptr<EpollChannel> channel, int event)
{
    return m_epoller->updateChannel(action, fd, channel, event);
}

void EpollWorker::addSession(std::shared_ptr<TcpSession> session)
{
    RWMutexType::WriteLock lock(m_rwMutex);
    m_sessions[session->id()] = session;
}

void EpollWorker::removeSession(unsigned long sessionId)
{
    RWMutexType::WriteLock lock(m_rwMutex);
    auto it = m_sessions.find(sessionId);
    if(it == m_sessions.end())
    {
        LOG_INFO(g_logger) << "unknown session";
        return ;
    }
    m_sessions.erase(it);
}

#endif
