#ifndef COROUTINE_SCHEDULER_H
#define COROUTINE_SCHEDULER_H

#include <memory>
#include <vector>
#include <atomic>
#include <list>

#include "../Mutex.h"
#include "Coroutine.h"
#include "../thread/ThreadPool.hpp"
#include "CoroutineWorkThread.h"

class Scheduler: public std::enable_shared_from_this<Scheduler>
{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

public:
    Scheduler(size_t threads = 1, const std::string& name = "");
    virtual ~Scheduler();

    inline const std::string& getName() const {return m_name;}

    void start();
    void stop();

    inline MutexType& mutex() { return m_mutex;}
    inline std::list<ThreadJobTarget>& allCoroutines() {return m_coroutines;}
    inline std::atomic<size_t>& activeThreadCount() { return m_activeThreadCount;}
    inline std::atomic<size_t>& idleThreadCount() { return m_idleThreadCount;}
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
            emptyBefore = scheduleNoLock(t, thread);
        }
    }

    /**
     * @brief Batch scheduling coroutines
    */
    template<class Iterator>
    void schedule(Iterator& begin, Iterator& end)
    {
        bool emptyBefore = false;
        {
            MutexType::Lock(m_mutex);
            while(begin != end)
            {
                emptyBefore = scheduleNoLock(&*begin, -1) || emptyBefore;
                ++begin;
            }
        }

    }

    std::ostream& dump(std::ostream& os);

    virtual bool stopping();
protected:

    inline bool hasIdleThreads() {  return m_idleThreadCount > 0;}

private:

    template<class Task>
    bool scheduleNoLock(Task fc, int thread)
    {
        bool emptyBefore = m_coroutines.empty();
        ThreadJobTarget ct(fc, thread);
        if(ct.coroutine || ct.cb)
        {
            m_coroutines.push_back(ct);
        }

        return emptyBefore;
    }

private:
    MutexType m_mutex;
    std::list<ThreadJobTarget> m_coroutines;    // coroutines ready to run
    Coroutine::ptr m_rootCoroutine;     // schedule coroutines when use_caller is true
    std::string m_name;  


protected:
    std::vector<int64_t> m_threadIds;
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activeThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = {0};
    bool m_isStopping = true;
    bool m_autoStop = false;
    int64_t m_rootThreadId = 0;     // main thread(use_caller)
    std::shared_ptr<CoroutineWorkThreadPool> m_threadPool;
};
#endif