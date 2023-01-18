#ifndef NETWORK_EVENT_EVENTTHREADLOOP_CPP
#define NETWORK_EVENT_EVENTTHREADLOOP_CPP

#include "EventThreadPool.h"
EventThreadPool::EventThreadPool(int size): m_threads(size), m_isQuited(false){}
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

#endif