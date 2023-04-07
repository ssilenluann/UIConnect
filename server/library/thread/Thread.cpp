#ifndef THREAD_THREAD_CPP
#define THREAD_THREAD_CPP

#include "Thread.h"
#include "../log/Logger.h"

static Logger::ptr g_logger = LOG_NAME("system");

Thread::Thread(const std::string &name): m_pid(0), m_name(name), m_isQuited(false), m_isEnd(false){}

Thread::~Thread()
{
    if(m_isQuited)
        return;

    quit();
}

void Thread::run()
{
    LOG_INFO(g_logger) << "thread start";

    m_thread.reset(new std::thread(m_func));
    m_thread->detach();
    m_threadId = m_thread->get_id();
    m_pid = ThreadUtil::GetThreadId();

}

void Thread::entry()
{
    LOG_INFO(g_logger) << "thread entry start";
    if(!m_func)
    {
        LOG_ERROR(g_logger) << "thread main func unvalid, thread entry exit";
        return;
    }

    m_func();
    LOG_ERROR(g_logger) << "thread entry function end";
    
    m_isEnd = true;
}

void Thread::bind(std::function<void()> func)
{
    m_func = func;
}

void Thread::quit()
{
    if(!m_isQuited) return;

    LOG_INFO(g_logger) << "thread start to quit";
    // here is a block call, thread safety    
    m_isQuited = true;
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this](){return m_isEnd == true;});     // wait thread entry func ended
    LOG_INFO(g_logger) << "thread quited";
}

#endif
