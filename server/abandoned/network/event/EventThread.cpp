#ifndef NETWORK_EVENT_EVENTTHREAD_CPP
#define NETWORK_EVENT_EVENTTHREAD_CPP

#include "EventThread.h"

#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

EventThread::EventThread(): Thread()
{
    m_loop.reset(new EventLoop());
    bind(std::bind(&EventLoop::loop, m_loop.get()));
}

EventThread::~EventThread()
{
    if(m_isQuited)
        return;

    quit();
}


void EventThread::run()
{
    Thread::run();
    Thread::detach();
}

void EventThread::quit()
{
    if(!m_isQuited)
        return;

    LOG_INFO(g_logger) << "event thread start to quit";
    m_loop->quit();
    
    Thread::quit();
    LOG_INFO(g_logger) << "event thread quited";
}

std::shared_ptr<EventLoop> EventThread::getLoop()
{
	return m_loop;	
}
#endif
