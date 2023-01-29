#ifndef NETWORK_EVENT_EVENTTHREAD_CPP
#define NETWORK_EVENT_EVENTTHREAD_CPP

#include "EventThread.h"

EventThread::EventThread(): m_isQuited(false){}

EventThread::~EventThread()
{
    
    if(m_isQuited)
        return;

    quit();
}

void EventThread::run()
{
    // TODO: LOG: thread job started
	m_loop.reset(new EventLoop());

    m_thread.reset(new std::thread([this](){m_loop->loop();}));
    m_thread->detach();
    m_threadId = m_thread->get_id();
    // TODO: LOG: thread job ended

}

void EventThread::quit()
{
    if(!m_isQuited)
        return;
        
    m_loop->quit();
    m_isQuited = true;
}

std::shared_ptr<EventLoop> EventThread::getLoop()
{
	return m_loop;	
}
#endif
