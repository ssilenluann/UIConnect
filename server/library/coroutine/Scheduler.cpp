#ifndef COROUTINE_SCHEDULER_CPP
#define COROUTINE_SCHEDULER_CPP

#include "../log/Logger.h"
#include "../thread/Thread.h"
#include "Scheduler.h"
#include "Coroutine.h"

static Logger::ptr g_logger = LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Coroutine* t_scheduler_coroutine = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name): m_name(name)
{
    LOG_ASSERT(threads > 0);

    if(use_caller)
    {
        Coroutine::GetTemp();
        --threads;

        LOG_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_rootCoroutine.reset(new Coroutine(std::bind(&Scheduler::run, this), 0, true));
        Thread::SetName(m_name);

        t_scheduler_coroutine = m_rootCoroutine.get();
        m_rootThreadId = ThreadUtil::GetThreadId();
        m_threadIds.push_back(m_rootThreadId);
    }
    else
    {
        m_rootThreadId = -1;
    }

    m_threadCount = threads;
}

Scheduler::~Scheduler()
{
    LOG_ASSERT(m_isStopping);
    if(GetThis() == this)
        t_scheduler = nullptr;
}

Scheduler *Scheduler::GetThis()
{
    return t_scheduler;
}

Coroutine *Scheduler::GetMainCoroutine()
{
    return t_scheduler_coroutine;
}

void Scheduler::start()
{
    MutexType::Lock lock(m_mutex);
    if(!m_isStopping)
    {
        return;
    }

    m_isStopping = false;
    LOG_ASSERT(m_threads.empty());

    m_threadIds.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i)
    {
        m_threads[i].reset(new Thread(m_name + "_" + std::to_string(i)));
        m_threads[i]->bind(std::bind(&Scheduler::run, this));
        m_threads[i]->run();
        m_threadIds.push_back(m_threads[i]->pid());
    }
}

void Scheduler::stop()
{
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

    LOG_ASSERT((m_rootThreadId == -1) ^ (GetThis() == this));

    m_isStopping = true;
    for(size_t i = 0; i < m_threadCount; ++i)
    {
        notify();
    }

    if(m_rootCoroutine) 
    {
        notify();
        if(!stopping()) m_rootCoroutine->call();
    }

    std::vector<Thread::ptr> threads;
    {
        MutexType::Lock lock(m_mutex);
        threads.swap(m_threads);
    }

    for(auto& t: threads)
        t->join();
}

void Scheduler::switchTo(int thread)
{
    LOG_ASSERT(Coroutine::GetTemp().get() != nullptr);
    if(GetThis() == this
        && (thread == -1 || thread == ThreadUtil::GetThreadId()))
        return;
    
    schedule(Coroutine::GetTemp(), thread);
    Coroutine::Yield2Hold();
}

std::ostream &Scheduler::dump(std::ostream &os)
{
    os << "[Scheduler name=" << m_name
       << " size=" << m_threadCount
       << " active_count=" << m_activeThreadCount
       << " idle_count=" << m_idleTHreadCount
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

void Scheduler::notify()
{
}

void Scheduler::run()
{
    LOG_DEBUG(g_logger) << m_name << "run";

    setThis();
    if(m_rootThreadId != ThreadUtil::GetThreadId())
        t_scheduler_coroutine = Coroutine::GetTemp().get();

    Coroutine::ptr idleCoroutine(new Coroutine(std::bind(&Scheduler::idle, this)));
    Coroutine::ptr cbCoroutine(new Coroutine(nullptr));

    CoroutineThread ct;
    for(;;)
    {
        ct.reset();
        bool notification = false;
        bool isActive = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_coroutines.begin();
            while(it != m_coroutines.end())
            {
                if(it->threadId = -1 && it->threadId != ThreadUtil::GetThreadId())
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

                ct = *it;
                it = m_coroutines.erase(it);
                ++m_activeThreadCount;
                isActive = true;
                break;
            }

            notification |= (it != m_coroutines.end());
        }

        if(notification)    notify();

        if((ct.coroutine 
                && (ct.coroutine->getState() != Coroutine::TERM && ct.coroutine->getState() != Coroutine::EXCEPT))
            || ct.cb)
        {
            runTask(ct);
        }
        else
        {
            if(isActive)
            {
                -- m_activeThreadCount;
                continue;
            }
            if(idleCoroutine->getState() == Coroutine::TERM)
            {
                LOG_INFO(g_logger) << "idle coroutine terminated";
                break;
            }

            ++ m_idleTHreadCount;
            idleCoroutine->swapIn();
            --m_idleTHreadCount;
            if(idleCoroutine->getState() != Coroutine::TERM && idleCoroutine->getState() != Coroutine::EXCEPT)
                idleCoroutine->setState(Coroutine::HOLD);
        }
    }    

}

bool Scheduler::stopping()
{
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_isStopping && m_coroutines.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle()
{
    LOG_INFO(g_logger) << "idle";
    while(!stopping())
        Coroutine::Yield2Hold();
}

void Scheduler::setThis()
{
}

void Scheduler::runTask(CoroutineThread& ct)
{
    Coroutine::ptr cbCoroutine(new Coroutine(std::function<void()>(nullptr)));
    if(ct.coroutine 
            && (ct.coroutine->getState() != Coroutine::TERM && ct.coroutine->getState() != Coroutine::EXCEPT))
    {
        cbCoroutine = ct.coroutine;
    }
    else if(ct.cb)
    {
        cbCoroutine->reset(ct.cb);
    }
    else
    {
        LOG_ASSERT_W(false, "task is not valid");
        return ;
    }

    cbCoroutine->swapIn();
    --m_activeThreadCount;

    if(ct.coroutine->getState() == Coroutine::READY)
        schedule(cbCoroutine);
    else if(ct.coroutine->getState() != Coroutine::TERM && ct.coroutine->getState() != Coroutine::EXCEPT)
        ct.coroutine->setState(Coroutine::HOLD);
}

SchedulerSwitcher::SchedulerSwitcher(std::shared_ptr<Scheduler> target)
{
    m_caller = Scheduler::GetThis();
    if(target)
        target->switchTo();
}

SchedulerSwitcher::~SchedulerSwitcher()
{
    if(m_caller)
        m_caller->switchTo();
}

#endif

