#ifndef COROUTINE_IOMANAGER_CPP
#define COROUTINE_IOMANAGER_CPP

#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "IOManager.h"
#include "../log/Logger.h"

static Logger::ptr g_logger = LOG_NAME("system");

IOManager::IOManager(size_t threadCount, const std::string &name, bool callerThreadJoinWorker)
    :Scheduler(threadCount, name, callerThreadJoinWorker)
{
    m_epfd = epoll_create(5000);
    LOG_ASSERT(m_epfd > 0);

    int ret = pipe(m_noticeFds);
    LOG_ASSERT(ret == 0);

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_noticeFds[0];

    ret = fcntl(m_noticeFds[0], F_SETFL, O_NONBLOCK);
    LOG_ASSERT(ret == 0);

    ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_noticeFds[0], &event);
    LOG_ASSERT(ret == 0);

    contextResize(32);

    m_timer = std::make_shared<TimerManager>();
    m_timer->setOnTimerInsertedAtFront([&](){ notice();});
    
}

IOManager::~IOManager()
{
    stop();
    close(m_epfd);
    close(m_noticeFds[0]);
    close(m_noticeFds[1]);

    m_fdContexts.clear();
}

bool IOManager::addEvent(int fd, EventType event, std::function<void()> cb)
{
    {
        RWMutexType::ReadLock lock(m_rwMutex);
        if((int)m_fdContexts.size() <= fd)
        {
            lock.unlock();
            RWMutexType::WriteLock wlock(m_rwMutex);
            contextResize(fd* 1.5);
        }
    }

    auto fd_ctx = m_fdContexts[fd];
    FdContext::MutexType::Lock lock(fd_ctx->mutex);
    if(fd_ctx->events & event)
    {
        LOG_ERROR(g_logger) << "fd has been added to epoll instance already, "
            << " fd = " << fd << ", target event = " << event
            << "current fd event = " << fd_ctx->events;
        
        LOG_ASSERT(false);
    }

    int op = fd_ctx->events == IOManager::EventType::NONE? EPOLL_CTL_ADD: EPOLL_CTL_MOD;
    epoll_event epEvent;
    memset(&epEvent, 0, sizeof(epoll_event));
    epEvent.events = EPOLLET | fd_ctx->events | event;
    epEvent.data.fd = fd;

    int ret = epoll_ctl(m_epfd, op, fd, &epEvent);
    if(ret < 0)
    {
        LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << epEvent.events << ") failed, errno = "
            << errno << ", info: " << strerror(errno);
        return false;
    }

    ++m_pendingEventCount;
    fd_ctx->events = (EventType)(fd_ctx->events | event) ;
    
    auto& event_ctx = fd_ctx->getContext(event);
    LOG_ASSERT(event_ctx.scheduler.lock() == nullptr
                && !event_ctx.coroutine
                && !event_ctx.cb);

    event_ctx.scheduler = Scheduler::GetScheduler();
    if(cb)
    {
        event_ctx.cb.swap(cb);  
    }
    else
    {
        // TODO
        event_ctx.coroutine = Coroutine::GetThreadCurrCoroutine();
        LOG_ASSERT_W(event_ctx.coroutine->getState() == Coroutine::State::EXEC
            , "coroutine state = " << event_ctx.coroutine->getState());
    }
    return true;
}

bool IOManager::delEvent(int fd, EventType event)
{
    RWMutexType::ReadLock lock(m_rwMutex);
    if((int)m_fdContexts.size() <= fd)
        return false;
    
    auto& fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock mlock(fd_ctx->mutex);
    if(fd_ctx->events & event == 0) 
        return false;
    
    EventType new_events = (EventType)(fd_ctx->events & ~event);
    int op = new_events == EventType::NONE? EPOLL_CTL_DEL: EPOLL_CTL_MOD;
    epoll_event epEvent;
    memset(&epEvent, 0, sizeof(epoll_event));
    epEvent.data.fd = fd;
    epEvent.events = new_events | EPOLLET;
    
    int ret = epoll_ctl(m_epfd, op, fd, &epEvent);
    if(ret < 0)
    {
        LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << epEvent.events << ") failed, errno = "
            << errno << ", info: " << strerror(errno);
        return false;
    }

    --m_pendingEventCount;
    fd_ctx->events = new_events;
    fd_ctx->resetContext(fd_ctx->getContext(event));
    return true;
}

bool IOManager::triggerAndCancelEvent(int fd, EventType event)
{
    RWMutexType::ReadLock lock(m_rwMutex);
    if((int)m_fdContexts.size() <= fd)
        return false;
    
    auto fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock mlock(fd_ctx->mutex);
    if(fd_ctx->events & event == 0)
        return false;
    
    EventType new_events = (EventType)(fd_ctx->events & ~event);
    int op = new_events == EventType::NONE? EPOLL_CTL_DEL: EPOLL_CTL_MOD;
    epoll_event epEvent;
    memset(&epEvent, 0, sizeof(epoll_event));
    epEvent.data.fd = fd;
    epEvent.events = new_events | EPOLLET;
    
    int ret = epoll_ctl(m_epfd, op, fd, &epEvent);
    if(ret < 0)
    {
        LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << epEvent.events << ") failed, errno = "
            << errno << ", info: " << strerror(errno);
        return false;
    }

    fd_ctx->triggerEvent(event);
    fd_ctx->events = new_events;
    --m_pendingEventCount;
    return true;
}

bool IOManager::triggerAndCancelEvent(int fd)
{
    RWMutexType::ReadLock lock(m_rwMutex);
    if((int)m_fdContexts.size() <= fd)
        return false;
    
    auto fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock mlock(fd_ctx->mutex);
    if(fd_ctx->events == 0)
        return false;
    
    epoll_event epEvent;
    memset(&epEvent, 0, sizeof(epoll_event));
    epEvent.data.fd = fd;
    epEvent.events = 0;
    
    int ret = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &epEvent);
    if(ret < 0)
    {
        LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << EPOLL_CTL_DEL << ", " << fd << ", " << epEvent.events << ") failed, errno = "
            << errno << ", info: " << strerror(errno);
        return false;
    }

    if(fd_ctx->events & EventType::READ)
    {
        fd_ctx->triggerEvent(EventType::READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events & EventType::WRITE)
    {
        fd_ctx->triggerEvent(EventType::WRITE);
        --m_pendingEventCount;
    }
    
    LOG_ASSERT(fd_ctx->events == 0);
    return true;
}

std::shared_ptr<IOManager> IOManager::GetThis()
{
    return std::dynamic_pointer_cast<IOManager>(Scheduler::GetScheduler());
}

void IOManager::start(std::shared_ptr<IOManager> manager)
{
    setScheduler(std::dynamic_pointer_cast<Scheduler>(manager));
    Scheduler::start();
}

TimerFunc::ptr IOManager::addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot)
{
    return m_timer->addTimer(cycle, cb, oneshot);
}

TimerFunc::ptr IOManager::addConditionTimer(uint64_t cycle, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool oneshot)
{
    return m_timer->addConditionTimer(cycle, cb, weak_cond, oneshot);
}

void IOManager::notice()
{
    if(!hasIdleThreads())   return;

    int ret = write(m_noticeFds[1], "T", 1);
    LOG_ASSERT(ret == 1);
}

bool IOManager::WorkCoroutineReadyToStop()
{
    uint64_t timeout = 0;
    return WorkCoroutineReadyToStop(timeout);
}

void IOManager::idle()
{
    LOG_DEBUG(g_logger) << "idle";
    const uint64_t MAX_EVENT_COUNT = 256;
    epoll_event* events = new epoll_event[MAX_EVENT_COUNT]();
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr)
        {
            delete[] ptr;
        }
    );

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

            cntAct = epoll_wait(m_epfd, events, MAX_EVENT_COUNT, (int)next_timeout);
            if(cntAct < 0 && errno == EINTR)
                continue;   // interrupted, conitnue
            if(cntAct < 0)
                LOG_DEBUG(g_logger) << "epoll error, instace fd = " << m_epfd 
                    << ", errno = " << errno << ", err info: " << strerror(errno);
            
            // whatever notified or timeout, break
            break;
        }while(true);

        // call timer
        std::vector<std::function<void()>> cbs;
        m_timer->listExpiredCb(cbs);
        // LOG_INFO(g_logger) << "expired timer count = " << cbs.size();
        for(auto& cb: cbs)
            schedule(cb);
        cbs.clear();

        for(int i = 0; i < cntAct; ++i)
        {
            epoll_event& event = events[i];
            if(event.data.fd == m_noticeFds[0])     // notice from schduler
            {
                char dummy[256] = {0};
                while(read(m_noticeFds[0], dummy, sizeof(dummy)) > 0);
                continue;
            }

            // process events
            RWMutexType::ReadLock lock(m_rwMutex);
            if(m_fdContexts.size() <= event.data.fd)
            {
                LOG_ERROR(g_logger) << "unknown fd event";
                continue;
            }
            auto fd_ctx = m_fdContexts[event.data.fd];
            lock.unlock();

            if(event.events & (EPOLLERR | EPOLLHUP))
            {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
            }

            int real_event = EventType::NONE;
            if(event.events & EPOLLIN) 
                triggerAndCancelEvent(event.data.fd, EventType::READ);
            if(event.events & EPOLLOUT)
                triggerAndCancelEvent(event.data.fd, EventType::WRITE);

        }

        // timer func and epoll event processe finish, swapout
        auto curCoroutine = Coroutine::GetThreadCurrCoroutine();
        auto raw_ptr = curCoroutine.get();
        curCoroutine.reset();

        raw_ptr->swapOut();
    }
}

bool IOManager::WorkCoroutineReadyToStop(uint64_t &timeout)
{
    timeout = m_timer->getNextTimer();
    return timeout == 0 && m_pendingEventCount == 0 && Scheduler::WorkCoroutineReadyToStop();
}

void IOManager::contextResize(size_t size)
{
    m_fdContexts.resize(size);
    for(size_t i = 0; i < m_fdContexts.size(); ++i)
    {
        if(m_fdContexts[i]) continue;
        m_fdContexts[i].reset(new FdContext);
        m_fdContexts[i]->fd = i;
    }
}

FdContext::EventContext& FdContext::getContext(IOManager::EventType event)
{
    switch(event)
    {
        case IOManager::EventType::READ:
            return read;
        case IOManager::EventType::WRITE:
            return write;
        default:
            LOG_ASSERT_W(false, "getContext");
    }

    throw std::invalid_argument("get context invalid event");
}

void FdContext::resetContext(EventContext &ctx)
{
    ctx.scheduler.reset();
    ctx.coroutine.reset();
    ctx.cb = nullptr;
}

void FdContext::triggerEvent(IOManager::EventType event)
{
    LOG_ASSERT(events & event);

    // event is about to trigger, remove it from events
    events = (IOManager::EventType)(events & ~event);
    EventContext& ctx = getContext(event);
    if(ctx.cb)
        ctx.scheduler.lock()->schedule(ctx.cb);
    else
        ctx.scheduler.lock()->schedule(ctx.coroutine);
    resetContext(ctx);
    return;
}

#endif

