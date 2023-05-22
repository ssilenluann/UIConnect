
#include "Thread.h"
#include "../log/Logger.h"

static thread_local std::string t_thread_name = "UNKNOWN";

static Logger::ptr g_logger = LOG_NAME("system");

Thread::Thread(const std::string &name): m_pid(-1), m_name(name), m_isQuited(false), m_isEnd(false){}

Thread::~Thread()
{
    if(m_isQuited)
        return;

    quit();
}

void Thread::run()
{
    LOG_INFO(g_logger) << "thread start, thread_name = " << m_name;

    m_thread.reset(new std::thread(std::bind(&Thread::entry, this)));
    m_threadId = m_thread->get_id();
    m_pid = ThreadUtil::GetThreadId();
    t_thread_name = m_name;
}

void Thread::entry()
{
    // LOG_INFO(g_logger) << "thread entry start, thread_name = " << m_name;
    if(!m_func)
    {
        LOG_ERROR(g_logger) << "thread main func unvalid, thread entry exit";
        return;
    }

    m_func();
    // LOG_ERROR(g_logger) << "thread entry function end, thread_name = " << m_name;
    
    m_isEnd = true;
    m_cv.notify_all();
}

const std::string& Thread::GetName()
{
    return t_thread_name;
}

void Thread::SetName(const std::string& name)
{
    if(name.empty())    return;
    
    t_thread_name = name;
}

void Thread::detach()
{
    if(m_thread->joinable())
        m_thread->detach();
    else
        LOG_ASSERT_W(false, "thread is not joinable");
}

void Thread::join()
{
    if(m_thread->joinable())
        m_thread->join();
    else
        LOG_ASSERT_W(false, "thread is not joinable");
}

void Thread::bind(std::function<void()> func)
{
    if(m_func)
    {
        LOG_ASSERT_W(false, "thread already has main func");
        return;
    }
    m_func = func;
}

void Thread::quit()
{
    if(m_isQuited) return;

    LOG_INFO(g_logger) << "thread start to quit";
    // here is a block call, thread safety    
    m_isQuited = true;
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this](){return m_isEnd == true;});     // wait thread entry func ended
    LOG_INFO(g_logger) << "thread quited";
}
