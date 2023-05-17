#ifndef NETWORK_EPOLL_EPOLLWORKER_CPP
#define NETWORK_EPOLL_EPOLLWORKER_CPP

#include "EpollWorker.h"

void EpollWorker::work()
{
    LOG_DEBUG(g_logger) << "idle";
    const uint64_t MAX_EVENT_COUNT = 256;

    for(;;)
    {
        uint64_t next_timeout = 0;
        if(WorkCoroutineReadyToStop(next_timeout))
        {
            LOG_DEBUG(g_logger) << "name" << getName()
                << " idle coroutine exit";
            break;
        }

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
            if(m_activeChannels[i]->fd() == m_noticeFds[0])     // notice from schduler
            {
                char dummy[256] = {0};
                while(read(m_noticeFds[0], dummy, sizeof(dummy)) > 0);
                continue;
            }

            // process events
            RWMutexType::ReadLock lock(m_rwMutex);
            if(m_channels.size() <= m_activeChannels[i]->fd())
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
            schedule(cb);
        cbs.clear();

        // timer func and epoll event processe finish, swapout
        auto curCoroutine = Coroutine::GetThreadCurrCoroutine();
        auto raw_ptr = curCoroutine.get();
        curCoroutine.reset();

        raw_ptr->swapOut();
    }
}

bool EpollWorker::readyToQuit()
{
    return m_isQuited && m_pendingEventCount == 0;
}

#endif
