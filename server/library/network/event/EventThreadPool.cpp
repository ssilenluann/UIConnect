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
    // TODO: LOG
    m_isQuited = false;

    for(int i = 0; i < m_size; i++)
    {
        m_threads[i].reset(new EventThread());
        m_idMap[m_threads[i]->id()] = m_threads[i];
        m_threads[i]->run();
    }
}

void EventThreadPool::quit()
{
    // TODO: LOG
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