#ifndef NETWORK_EVENT_EVENTLOOP_CPP
#define NETWORK_EVENT_EVENTLOOP_CPP
#include "EventLoop.h"

#include <algorithm>

EventLoop::EventLoop(SOCKET sock, std::thread::id id):m_epoll(sock), m_threadId(id), m_isQuiting(false), m_isRunning(false){}

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
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isRunning = true;
    }
    
    //TODO: LOG: event loop start loop

    while(m_isQuiting)
    {
        bool retp = m_epoll.poll(m_activeChannels);
        if(!retp)
        {
            // TODO: LOG
            break;
        }

        for(auto& channel: m_activeChannels)
            channel->handleEvent();
        
        m_activeChannels.clear();

		// do tasks
		doTasks();
    }

    //TODO: LOG: event loop end loop
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isRunning = false;
        m_cv.notify_all();
    }

}

void EventLoop::quit()
{
    // TODO: LOG: start quit
    std::unique_lock<std::mutex> lock(m_mutex);
    m_isQuiting = true;
    m_cv.wait(lock, [this]() {return m_isRunning == false;});
    
    //TODO: LOG: end quit
}

bool EventLoop::isInLoopThread()
{
    return std::this_thread::get_id() == m_threadId;
}

bool EventLoop::updateChannel(SOCKET fd, std::shared_ptr<TcpChannel> pChannel, int action, int type)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if(!checkChannelInLoop(pChannel) || !isInLoopThread()) 
    {
        // TODO: LOG
        return false;
    }

    return m_epoll.ctrl(fd, action, type) >= 0;
}

bool EventLoop::checkChannelInLoop(std::shared_ptr<TcpChannel>& pChannel)
{
    if(pChannel == nullptr)
        return false;

    auto pChannelLoop = pChannel->eventLoop().lock();
    if(pChannelLoop == nullptr || pChannelLoop.get() != this)
    {
        // TODO: LOG
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
#endif
