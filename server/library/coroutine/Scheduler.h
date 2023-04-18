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

class Scheduler: public std::enable_shared_from_this<Scheduler>
{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

public:
    Scheduler(size_t threads = 1, const std::string& name = "");
    virtual ~Scheduler();

    void work();
    virtual void idle();
    void runCoroutineTask(CoroutineJobTarget& job);

    void start();
    void stop();
    virtual bool ReadyToStop();
    std::ostream& dump(std::ostream& os);
    // new task comming, notice all threads
    virtual void notice();
 
    inline const std::string& getName() const {return m_name;}
    inline MutexType& mutex() { return m_mutex;}
    inline std::list<CoroutineJobTarget>& allCoroutines() {return m_coroutineTasks;}
    inline std::atomic<size_t>& activeThreadCount() { return m_activeThreadCount;}
    inline std::atomic<size_t>& idleThreadCount() { return m_idleThreadCount;}
    inline bool hasIdleThreads() {  return m_idleThreadCount > 0;}

    static std::shared_ptr<Scheduler> GetScheduler();

protected:
    virtual void setScheduler(std::shared_ptr<Scheduler> scheduler);
    virtual void setScheduler();

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

private:

    template<class Task>
    bool scheduleNoLock(Task fc, int thread)
    {
        bool emptyBefore = m_coroutineTasks.empty();
        CoroutineJobTarget ct(fc, thread);
        if(ct.coroutine || ct.cb)
        {
            m_coroutineTasks.push_back(ct);
        }

        return emptyBefore;
    }

protected:
    bool m_isStopping = true;
    bool m_autoStop = false;
    MutexType m_mutex;
    std::string m_name;  
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activeThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = {0};
    std::shared_ptr<ThreadPool<Thread>> m_workThreadPool;
    std::vector<int64_t> m_threadIds;
    std::list<CoroutineJobTarget> m_coroutineTasks;    // coroutines ready to run
};

#endif