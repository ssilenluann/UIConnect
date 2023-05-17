#ifndef COROUTINE_SCHEDULER_H
#define COROUTINE_SCHEDULER_H

#include <memory>
#include <vector>
#include <atomic>
#include <list>

#include "../Mutex.h"
#include "Coroutine.h"

#include "../thread/Thread.h"
#include "../thread/ThreadPool.hpp"
#include <memory>
#include <functional>

// thread work class, coule be assigned to a thread, then thread will call it's function or coroutine
struct CoroutineJobTarget
{
    CoroutineJobTarget(Coroutine::ptr c, int thread): coroutine(c), workThreadId(thread){}
    CoroutineJobTarget(std::function<void()> f, int thread): cb(f), workThreadId(thread) {} 
    CoroutineJobTarget(): workThreadId(-1) {}
    void reset()  
    {
        coroutine.reset();
        cb = nullptr;
        workThreadId = -1;
    }
    Coroutine::ptr coroutine;
    std::function<void()> cb;
    int workThreadId; 

};


template<class T>
class Scheduler;

template<class T>
class SchedulerThreadPool: public ThreadPool<T>
{
public:

    SchedulerThreadPool(std::function<void(std::shared_ptr<T>)> funcWithThread, std::shared_ptr<Scheduler<T>>sc, int threadCount = 4)
    {
        ThreadPool<T>::m_size = threadCount;
        for(int i = 0; i < threadCount; i++)
        {
            ThreadPool<T>::m_threads[i].reset(new T(sc));
            ThreadPool<T>::m_threads[i]->bind(funcWithThread);
        }
    }
};


template<class T>
class Scheduler: public std::enable_shared_from_this<Scheduler<T>>
{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

public:

    Scheduler(size_t threads = 1, const std::string& name = "", bool callerThreadJoinWorker = true) 
        : m_name(name), m_threadCount(threads)
    {
    }

    virtual ~Scheduler()
    {

    }

    void work(std::shared_ptr<T> worker)
    {
        Coroutine::Init();
        
        // create work coroutine
        std::shared_ptr<Coroutine> workCoroutine = std::make_shared<Coroutine>(std::bind(&T::work, worker));

        // LOG_DEBUG(g_logger) << m_name << " starts to run coroutine";

        CoroutineJobTarget job;
        for(;;)
        {
            job.reset();
            {
                
                MutexType::Lock lock(m_mutex);
                auto it = m_coroutineTasks.begin();
                while(it != m_coroutineTasks.end())
                {
                    // tasks has been specifed, check current is the specified thread
                    if(it->workThreadId != -1 && it->workThreadId != ThreadUtil::GetThreadId())
                    {
                        ++it;
                        continue;
                    }

                    // task is already running
                    if(it->coroutine && it->coroutine->getState() == Coroutine::EXEC)
                    {
                        ++it; 
                        continue;
                    }

                    job = *it;
                    it = m_coroutineTasks.erase(it);
                    break;
                }

            }

            if((job.coroutine 
                    && (job.coroutine->getState() != Coroutine::TERM && job.coroutine->getState() != Coroutine::EXCEPT))
                || job.cb)
            {
                runCoroutineTask(job);
            }
            else
            {
                if(workCoroutine->getState() == Coroutine::TERM) break;

                workCoroutine->swapIn();

                if (workCoroutine->getState() != Coroutine::TERM && workCoroutine->getState() != Coroutine::EXCEPT)
                    workCoroutine->setState(Coroutine::HOLD);
            }
        } 
    }

    void runCoroutineTask(CoroutineJobTarget& job)
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
            // LOG_ASSERT_W(false, "task is not valid");
            return ;
        }

        // LOG_DEBUG(g_logger) << "task coroutine swap in, coroutine_id = " << cbCoroutine->getId();
        cbCoroutine->swapIn();
        // LOG_DEBUG(g_logger) << "task coroutine swap out, coroutine_id = " << cbCoroutine->getId();

        if(cbCoroutine->getState() == Coroutine::READY)
            schedule(cbCoroutine);
        else if(cbCoroutine->getState() != Coroutine::TERM && cbCoroutine->getState() != Coroutine::EXCEPT)
            cbCoroutine->setState(Coroutine::HOLD);          
    }

    void start()
    {
        // setScheduler();

        MutexType::Lock lock(m_mutex);
        if(!m_isStopping) return;
        m_isStopping = false;

        auto schedulerMainFunc = 
            std::bind(&Scheduler<T>::work, 
                this,
                std::placeholders::_1);

        // create thread pool and specify each thread's entry function   
        m_workThreadPool.reset(new SchedulerThreadPool<T>(
            schedulerMainFunc, this->shared_from_this(), m_threadCount));

        m_workThreadPool->start();
        m_workThreadPool->detach();
    }

    void stop()
    {
        if(m_isStopping)    return;

        m_isStopping = true;
        m_workThreadPool->quit();
    }

    bool isStopping() { return m_isStopping;}
    // std::ostream& dump(std::ostream& os)
    // {
    //     os << "[Scheduler name=" << m_name
    //     << " size=" << m_threadCount
    //     // << " active_count=" << m_activeThreadCount
    //     // << " idle_count=" << m_idleThreadCount
    //     << " WorkCoroutineReadyToStop=" << m_isStopping
    //     << " ]" << std::endl << "    ";
    //     for(size_t i = 0; i < m_threadIds.size(); ++i) {
    //         if(i) {
    //             os << ", ";
    //         }
    //         os << m_threadIds[i];
    //     }
    //     return os;
    // }

    // new task comming, notice all threads
    virtual void notice()
    {
    }

    inline const std::string& getName() const {return m_name;}
    inline MutexType& mutex() { return m_mutex;}
    inline std::list<CoroutineJobTarget>& allCoroutines() {return m_coroutineTasks;}

    static std::shared_ptr<Scheduler<T>> GetScheduler();

    void waitForQuit()
    {
        m_workThreadPool->join();
    }

    bool allowWorkerQuit() { return m_isStopping && m_coroutineTasks.empty();}

public:
    /**
     * @brief schedule coroutine
     * @param[in] t coroutine or function
     * @param[in] specify witch thread to perform tasks
    */
    template<class Task>
    void schedule(Task t, int thread = -1)
    {
        bool emptyBefore = false;
        {
            MutexType::Lock lock(m_mutex);
            emptyBefore = m_coroutineTasks.empty();
            CoroutineJobTarget ct(t, thread);
            if(ct.coroutine || ct.cb)
            {
                m_coroutineTasks.push_back(ct);
            }
        }

        if(emptyBefore) notice();
    }

protected:
    bool m_isStopping = true;
    MutexType m_mutex;
    std::string m_name;  
    size_t m_threadCount = 0;
    std::shared_ptr<SchedulerThreadPool<T>> m_workThreadPool;
    std::list<CoroutineJobTarget> m_coroutineTasks;    // coroutines ready to run
    ConditionMutex m_cv;
};


class SchedulerWorker: public Thread, private std::enable_shared_from_this<SchedulerWorker>
{
public:
    SchedulerWorker(const std::string &name = ""): Thread(){}
    SchedulerWorker(std::shared_ptr<Scheduler<SchedulerWorker>> scheduler): m_scheduler(scheduler)
    {
    }

    virtual void work()
    {
    
        // LOG_INFO(g_logger) << "idle";
        while(!readyToQuit())
            Coroutine::Yield2Hold();

    }
    virtual bool readyToQuit()
    {
        if(m_scheduler.expired())   return true;
        return m_scheduler.lock()->isStopping() && m_scheduler.lock()->allowWorkerQuit();        
    }
    
    void bind(std::function<void(std::shared_ptr<SchedulerWorker>)> funcWithThread)
    {
        m_schedulerFunc = funcWithThread;
    }

    void entry() override
    {
        // LOG_INFO(g_logger) << "thread entry start, thread_name = " << m_name;
        if(!m_schedulerFunc)
        {
            // LOG_ERROR(g_logger) << "thread main func unvalid, thread entry exit";
            return;
        }

        m_schedulerFunc(shared_from_this());
        // LOG_ERROR(g_logger) << "thread entry function end, thread_name = " << m_name;
        
        m_isEnd = true;
        m_cv.notify_all();
    }
protected:
    std::weak_ptr<Scheduler<SchedulerWorker>> m_scheduler;
    std::function<void(std::shared_ptr<SchedulerWorker>)> m_schedulerFunc;
};

#endif