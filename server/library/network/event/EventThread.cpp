#ifndef NETWORK_EVENT_EVENTTHREAD_CPP
#define NETWORK_EVENT_EVENTTHREAD_CPP

#include "EventThread.h"

#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

EventThread::EventThread(): m_isQuited(false){}

EventThread::~EventThread()
{
    
    if(m_isQuited)
        return;

    quit();
}

void EventThread::run()
{
    LOG_INFO(g_logger) << "event thread start";
	m_loop.reset(new EventLoop());

    m_thread.reset(new std::thread(std::mem_fn(&EventLoop::loop), m_loop.get()));
    m_thread->detach();
    m_threadId = m_thread->get_id();

}

void EventThread::quit()
{
    
    if(!m_isQuited)
        return;

    LOG_INFO(g_logger) << "event thread start to quit";
    // here is a block call, thread safety    
    m_loop->quit();
    m_isQuited = true;
    LOG_INFO(g_logger) << "event thread quited";
}

std::shared_ptr<EventLoop> EventThread::getLoop()
{
	return m_loop;	
}
#endif
