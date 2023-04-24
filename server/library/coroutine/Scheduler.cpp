#ifndef COROUTINE_SCHEDULER_CPP
#define COROUTINE_SCHEDULER_CPP

#include "../log/Logger.h"
#include "../thread/Thread.h"
#include "Scheduler.h"
#include "Coroutine.h"
#include "../hook/Hook.h"
#include <unistd.h>

static Logger::ptr g_logger = LOG_NAME("system");

static std::weak_ptr<Scheduler> t_scheduler;

Scheduler::Scheduler(size_t threads, const std::string &name)
    : m_name(name), m_threadCount(threads)
{
    LOG_ASSERT(threads > 0);
}

Scheduler::~Scheduler()
{
    LOG_ASSERT(m_isStopping);
}

void Scheduler::work()
{
    Hook::SetHookEnable(true);

    Coroutine::Init();
    
    // create idle coroutine
    std::shared_ptr<Coroutine> idleCoroutine = std::make_shared<Coroutine>(std::bind(&Scheduler::idle, this));

    LOG_DEBUG(g_logger) << m_name << " starts to run coroutine";

    CoroutineJobTarget job;
    for(;;)
    {
        job.reset();
        bool notification = false;
        {
            
            MutexType::Lock lock(m_mutex);
            auto it = m_coroutineTasks.begin();
            while(it != m_coroutineTasks.end())
            {
                if(it->workThreadId != -1 && it->workThreadId != ThreadUtil::GetThreadId())
                {
                    ++it;
                    notification = true;
                    continue;
                }

                LOG_ASSERT(it->coroutine || it->cb);
                if(it->coroutine && it->coroutine->getState() == Coroutine::EXEC)
                {
                    ++it; 
                    continue;
                }

                job = *it;
                it = m_coroutineTasks.erase(it);
                break;
            }

            notification |= (it != m_coroutineTasks.end());
        }

        if((job.coroutine 
                && (job.coroutine->getState() != Coroutine::TERM && job.coroutine->getState() != Coroutine::EXCEPT))
            || job.cb)
        {
            runCoroutineTask(job);
        }
        else
        {
            if(idleCoroutine->getState() == Coroutine::TERM)
            {
                LOG_INFO(g_logger) << "idle coroutine terminated";
                break;
            }

            ++ m_idleThreadCount;
            idleCoroutine->swapIn();
            -- m_idleThreadCount;

            if (idleCoroutine->getState() != Coroutine::TERM && idleCoroutine->getState() != Coroutine::EXCEPT)
                idleCoroutine->setState(Coroutine::HOLD);
        }
    } 
}

void Scheduler::setScheduler(std::shared_ptr<Scheduler> scheduler)
{
    if (t_scheduler.lock() == nullptr)
        t_scheduler = scheduler;
}

void Scheduler::setScheduler() 
{
    if (t_scheduler.lock() == nullptr)
        t_scheduler = shared_from_this();        
}

void Scheduler::idle()
{
    LOG_INFO(g_logger) << "idle";
    while(!ReadyToStop())
        Coroutine::Yield2Hold();
}

void Scheduler::runCoroutineTask(CoroutineJobTarget& job)
{
    Coroutine::ptr cbCoroutine(new Coroutine(std::function<void()>(nullptr)));
    if(job.coroutine 
            && (job.coroutine->getState() != Coroutine::TERM && job.coroutine->getState() != Coroutine::EXCEPT))
    {
        cbCoroutine = job.coroutine;
    }
    else if(job.cb)
    {
        cbCoroutine->reset(job.cb);
    }
    else
    {
        LOG_ASSERT_W(false, "task is not valid");
        return ;
    }

    // LOG_DEBUG(g_logger) << "task coroutine swap in, coroutine_id = " << cbCoroutine->getId();
    cbCoroutine->swapIn();
    // LOG_DEBUG(g_logger) << "task coroutine swap out, coroutine_id = " << cbCoroutine->getId();
    --m_activeThreadCount;

    if(cbCoroutine->getState() == Coroutine::READY)
        schedule(cbCoroutine);
    else if(cbCoroutine->getState() != Coroutine::TERM && cbCoroutine->getState() != Coroutine::EXCEPT)
        cbCoroutine->setState(Coroutine::HOLD);  
}

void Scheduler::start()
{
    setScheduler();

    MutexType::Lock lock(m_mutex);
    if(!m_isStopping)
    {
        return;
    }

    m_isStopping = false;

    // create thread pool and specify each thread's entry function   
    m_workThreadPool = std::make_shared<ThreadPool<Thread>>(std::bind(&Scheduler::work, this), m_threadCount);
    m_workThreadPool->start();
    m_workThreadPool->detach();
}

void Scheduler::stop()
{
    // LOG_INFO(g_logger) << "scheduler " << m_name << " start to quit";
    m_autoStop = true;
    if( m_threadCount == 0
        && (Coroutine::GetThreadRootCoroutine()->getState() == Coroutine::TERM
            || Coroutine::GetThreadRootCoroutine()->getState() == Coroutine::INIT)
    )
    {
        LOG_INFO(g_logger) << this << "stopped";
        m_isStopping = true;

        if(ReadyToStop())  return;
    }

    m_isStopping = true;
    m_workThreadPool->quit();

    // LOG_INFO(g_logger) << "scheduler " << m_name << " quited";
}

std::ostream &Scheduler::dump(std::ostream &os)
{
    os << "[Scheduler name=" << m_name
       << " size=" << m_threadCount
       << " active_count=" << m_activeThreadCount
       << " idle_count=" << m_idleThreadCount
       << " ReadyToStop=" << m_isStopping
       << " ]" << std::endl << "    ";
    for(size_t i = 0; i < m_threadIds.size(); ++i) {
        if(i) {
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}

void Scheduler::notice()
{
    LOG_INFO(g_logger) << "notice";
}

std::shared_ptr<Scheduler> Scheduler::GetScheduler()
{
    return t_scheduler.lock();
}

bool Scheduler::ReadyToStop()
{
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_isStopping && m_coroutineTasks.empty();
}


#endif

