#ifndef NETWORK_EPOLL_EPOLLWORKER_H
#define NETWORK_EPOLL_EPOLLWORKER_H

#include "../../thread/Thread.h"
#include "../../coroutine/SchedulerWorker.h"
#include "../../TimerFunc.h"
#include "Epoll.h"
class EpollWorker: public SchedulerWorker
{
public:
    virtual void work() override;
    virtual bool readyToQuit() override;

private:
    std::atomic<size_t> m_pendingEventCount = {0};
    std::vector<EpollChannel::ptr> m_channels;
    std::vector<EpollChannel::ptr> m_activeChannels;
    TimerManager::ptr m_timer;
    Epoll::ptr m_epoller;    
};
#endif