#ifndef COROUTINE_COROUTINEWORKTHREAD_CPP
#define COROUTINE_COROUTINEWORKTHREAD_CPP

#include "CoroutineWorkThread.h"
#include "../log/Logger.h"
#include "Scheduler.h"

static Logger::ptr g_logger = LOG_NAME("system");
static std::weak_ptr<Scheduler> g_scheduler;
CoroutineWorkThread::CoroutineWorkThread(std::shared_ptr<Scheduler> scheduler, const std::string& name)
    : Thread(name)
{
    m_scheduler = scheduler;
    bind(std::bind(&CoroutineWorkThread::runCoroutine, this));
    LOG_DEBUG(g_logger) << "create idle coroutine";
    m_idleCoroutine.reset(new Coroutine(std::bind(&CoroutineWorkThread::idle, this)));
    if(g_scheduler.lock() == nullptr)
        g_scheduler = scheduler;
}


CoroutineWorkThread::~CoroutineWorkThread() 
{
}

void CoroutineWorkThread::run()
{
    Thread::run();
    Thread::deteach();
}

void CoroutineWorkThread::runCoroutine()
{
    Coroutine::Init();

    LOG_DEBUG(g_logger) << m_name << " starts to run coroutine";

    ThreadJobTarget job;
    for(;;)
    {
        job.reset();
        bool notification = false;
        bool isActive = false;
        Scheduler::ptr scheduler = m_scheduler.lock();
        {
            
            MutexType::Lock lock(scheduler->mutex());
            auto it = scheduler->allCoroutines().begin();
            while(it != scheduler->allCoroutines().end())
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
                it = scheduler->allCoroutines().erase(it);
                ++scheduler->activeThreadCount();
                isActive = true;
                break;
            }

            notification |= (it != scheduler->allCoroutines().end());
        }

        if((job.coroutine 
                && (job.coroutine->getState() != Coroutine::TERM && job.coroutine->getState() != Coroutine::EXCEPT))
            || job.cb)
        {
            runCoroutineTask(job);
        }
        else
        {
            if(isActive)
            {
                -- scheduler->activeThreadCount();
                continue;
            }
            if(m_idleCoroutine->getState() == Coroutine::TERM)
            {
                LOG_INFO(g_logger) << "idle coroutine terminated";
                break;
            }

            ++ scheduler->idleThreadCount();
            m_idleCoroutine->swapIn();
            -- scheduler->idleThreadCount();
            if(m_idleCoroutine->getState() != Coroutine::TERM && m_idleCoroutine->getState() != Coroutine::EXCEPT)
                m_idleCoroutine->setState(Coroutine::HOLD);
        }
    }    

}

void CoroutineWorkThread::runCoroutineTask(ThreadJobTarget& job)
{
    auto scheduler = m_scheduler.lock();
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

    LOG_DEBUG(g_logger) << "task coroutine swap in, coroutine_id = " << cbCoroutine->getId();
    cbCoroutine->swapIn();
    LOG_DEBUG(g_logger) << "task coroutine swap out, coroutine_id = " << cbCoroutine->getId();
    --scheduler->activeThreadCount();

    if(cbCoroutine->getState() == Coroutine::READY)
        scheduler->schedule(cbCoroutine);
    else if(cbCoroutine->getState() != Coroutine::TERM && cbCoroutine->getState() != Coroutine::EXCEPT)
        cbCoroutine->setState(Coroutine::HOLD);  
}

void CoroutineWorkThread::idle()
{
    LOG_INFO(g_logger) << "idle";
    auto scheduler = m_scheduler.lock();
    while(!scheduler->stopping())
        Coroutine::Yield2Hold();
}

std::shared_ptr<Scheduler> CoroutineWorkThread::GetScheduler()
{
    return g_scheduler.lock();
}

// TODO: add request
#endif