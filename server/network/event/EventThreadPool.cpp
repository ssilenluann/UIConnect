#ifndef NETWORK_EVENT_EVENTTHREADLOOP_CPP
#define NETWORK_EVENT_EVENTTHREADLOOP_CPP

#include "EventThreadPool.h"
EventThreadPool::EventThreadPool(int size): m_size(size), m_isQuited(false), m_threads(size){}

EventThreadPool::~EventThreadPool()
{
    if(m_isQuited)
        return;
    
    quit();
}

void EventThreadPool::start()
{
    m_isQuited = false;

    for(auto& thread: m_threads)
    {
        thread.reset(new EventThread());
        thread->run();
    }
}

void EventThreadPool::quit()
{
    for(auto& thread: m_threads)
    {
        thread->quit();
    }

    m_isQuited = true;
}

std::shared_ptr<EventLoop> EventThreadPool::getNextLoop()
{
	static int no = 0;
	no %= m_size;
	std::shared_ptr<EventLoop> loop = m_threads[no]->getLoop();
	no ++;
	return loop;
}
#endif
