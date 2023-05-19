#ifndef NETWORK_EVENT_EVENTLOOP_CPP
#define NETWORK_EVENT_EVENTLOOP_CPP
#include "EventLoop.h"
#include "../TcpSession.h"
#include "../../log/Logger.h"

#include <algorithm>

static Logger::ptr g_logger = LOG_NAME("system");

EventLoop::EventLoop(std::thread::id id)
: m_threadId(id), m_isQuiting(false), m_isRunning(false), m_timer(new TimerManager()){}

EventLoop::~EventLoop()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_isQuiting && !m_isRunning)
            return;
    }

    quit();
}

void EventLoop::loop()
{
    m_threadId = std::this_thread::get_id();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isRunning = true;
    }
    
    LOG_INFO(g_logger) << "event loop started";
    

    while(!m_isQuiting && m_isRunning)
    {
        static const int MAX_TIMEOUT = 3000;
        int next_timeout = (next_timeout > MAX_TIMEOUT || next_timeout == 0)
            ? MAX_TIMEOUT : next_timeout;

        bool retp = m_epoll.poll(m_activeChannels, next_timeout);
        if(!retp)
        {
            LOG_FMT_ERROR(g_logger, "epoll error, event loop will quit, errno = %d", errno);
            break;
        }

        for(auto& channel: m_activeChannels)
            channel->handleEvent();
        
        m_activeChannels.clear();

		// do tasks
		doTasks();

        // do timers
        std::vector<std::function<void()>> cbs;
        m_timer->listExpiredCb(cbs);
        // LOG_INFO(g_logger) << "expired timer count = " << cbs.size();
        for(auto& cb: cbs)
            cb();

        cbs.clear();        
    }

    LOG_INFO(g_logger) << "event loop end";
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sessions.clear();
        m_isRunning = false;
        m_cv.notify_all();
    }

}

void EventLoop::quit()
{
    LOG_INFO(g_logger) << "event loop start quit";
    std::unique_lock<std::mutex> lock(m_mutex);
    m_isQuiting = true;

    // waitting for main loop finished
    m_cv.wait(lock, [this]() {return m_isRunning == false;});

    m_epoll.close();

    LOG_INFO(g_logger) << "event loop quited";
}

bool EventLoop::isInLoopThread()
{
    return std::this_thread::get_id() == m_threadId;
}

bool EventLoop::updateChannel(SOCKET fd, std::shared_ptr<TcpChannel> pChannel, int action, uint32_t event)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if(!checkChannelInLoop(pChannel) || !isInLoopThread()) 
    {
        LOG_INFO(g_logger) << "channel is not belong to this thread or event loop";
        return false;
    }

    return m_epoll.updateChannel(action, fd, pChannel, event);
}

bool EventLoop::checkChannelInLoop(std::shared_ptr<TcpChannel>& pChannel)
{
    if(pChannel == nullptr)
        return false;

    auto pChannelLoop = pChannel->eventLoop().lock();
    if(pChannelLoop == nullptr || pChannelLoop.get() != this)
    {
        return false;
    }
    
    return true;
}

void EventLoop::addTask(TASK_FUNCTION task)
{
	if(isInLoopThread())
	{
		task();
	}
	else
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tasks.push_back(task);
	}
		
}

void EventLoop::doTasks()
{
	TASK_LIST functions;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		std::swap(functions, m_tasks);
		m_tasks.clear();
	}

	for(auto& function : functions)
		function();
		
}

void EventLoop::addSession(std::shared_ptr<TcpSession> session)
{
    addTask([session, this]()
    {
        if(!session->init())
        {
            // TOOD: log
            return;
        }

        m_sessions.emplace(session->id(), std::move(session));
    });
}

void EventLoop::removeSession(unsigned long sessionId)
{
    addTask([&]()
    {
        m_sessions.erase(sessionId);
    });
}

TimerFunc::ptr EventLoop::addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot)
{
    return m_timer->addTimer(cycle, cb, oneshot);
}

#endif
