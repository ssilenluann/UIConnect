#ifndef COROUTINE_SCHEDULER_CPP
#define COROUTINE_SCHEDULER_CPP

#include "../log/Logger.h"
#include "../thread/Thread.h"
#include "Scheduler.h"
#include "Coroutine.h"

static Logger::ptr g_logger = LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Coroutine* t_scheduler_coroutine = nullptr;

Scheduler::Scheduler(size_t threads, const std::string &name)
    : m_name(name), m_threadPool(std::make_shared<CoroutineWorkThreadPool>(threads))
{
    LOG_ASSERT(threads > 0);

    m_rootThreadId = -1;
    
    m_threadCount = threads;
}

Scheduler::~Scheduler()
{
    LOG_ASSERT(m_isStopping);
}

void Scheduler::start()
{
    MutexType::Lock lock(m_mutex);
    if(!m_isStopping)
    {
        return;
    }

    m_isStopping = false;

    m_threadPool->start(shared_from_this(), m_name);
}

void Scheduler::stop()
{
    LOG_INFO(g_logger) << "scheduler " << m_name << " start to quit";
    m_autoStop = true;
    if(m_rootCoroutine
        && m_threadCount == 0
        && (m_rootCoroutine->getState() == Coroutine::TERM
            || m_rootCoroutine->getState() == Coroutine::INIT)
    )
    {
        LOG_INFO(g_logger) << this << "stopped";
        m_isStopping = true;

        if(stopping())  return;
    }

    m_isStopping = true;
    m_threadPool->quit();

    LOG_INFO(g_logger) << "scheduler " << m_name << " quited";
}

std::ostream &Scheduler::dump(std::ostream &os)
{
    os << "[Scheduler name=" << m_name
       << " size=" << m_threadCount
       << " active_count=" << m_activeThreadCount
       << " idle_count=" << m_idleThreadCount
       << " stopping=" << m_isStopping
       << " ]" << std::endl << "    ";
    for(size_t i = 0; i < m_threadIds.size(); ++i) {
        if(i) {
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}

bool Scheduler::stopping()
{
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_isStopping && m_coroutines.empty() && m_activeThreadCount == 0;
}


#endif

