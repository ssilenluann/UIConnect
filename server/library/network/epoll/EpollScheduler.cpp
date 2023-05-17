#ifndef NETWORK_EPOLL_EPOLLSCHEDULER_CPP
#define NETWORK_EPOLL_EPOLLSCHEDULER_CPP

#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "EpollScheduler.h"
#include "../log/Logger.h"

static Logger::ptr g_logger = LOG_NAME("system");

EpollScheduler::EpollScheduler(size_t threadCount, const std::string &name, bool callerThreadJoinWorker)
    :Scheduler(threadCount, name, callerThreadJoinWorker), m_epoller(new Epoll())
{
    
    LOG_ASSERT(m_epoller->fd() > 0);

    int ret = pipe(m_noticeFds);
    LOG_ASSERT(ret == 0);

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_noticeFds[0];

    ret = fcntl(m_noticeFds[0], F_SETFL, O_NONBLOCK);
    LOG_ASSERT(ret == 0);

    ret = epoll_ctl(m_epoller->fd(), EPOLL_CTL_ADD, m_noticeFds[0], &event);
    LOG_ASSERT(ret == 0);

    m_timer = std::make_shared<TimerManager>();
    m_timer->setOnTimerInsertedAtFront([&](){ notice();});
    
}

EpollScheduler::~EpollScheduler()
{
    stop();
    close(m_noticeFds[0]);
    close(m_noticeFds[1]);
}

std::shared_ptr<EpollScheduler> EpollScheduler::GetThis()
{
    return std::dynamic_pointer_cast<EpollScheduler>(Scheduler::GetScheduler());
}

void EpollScheduler::start(std::shared_ptr<EpollScheduler> manager)
{
    setScheduler(std::dynamic_pointer_cast<Scheduler>(manager));
    Scheduler::start();
}

TimerFunc::ptr EpollScheduler::addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot)
{
    return m_timer->addTimer(cycle, cb, oneshot);
}

TimerFunc::ptr EpollScheduler::addConditionTimer(uint64_t cycle, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool oneshot)
{
    return m_timer->addConditionTimer(cycle, cb, weak_cond, oneshot);
}

bool EpollScheduler::updateChannel(int action, int fd, std::shared_ptr<EpollChannel> channel, int event)
{
    return m_epoller->updateChannel(action, fd, channel, event);
}

void EpollScheduler::notice()
{
    if(!hasIdleThreads())   return;

    int ret = write(m_noticeFds[1], "T", 1);
    LOG_ASSERT(ret == 1);
}


#endif

