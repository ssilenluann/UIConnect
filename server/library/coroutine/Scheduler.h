#ifndef COROUTINE_SCHEDULER_H
#define COROUTINE_SCHEDULER_H

#include <memory>
#include <vector>
#include <atomic>
#include <list>

#include "../Mutex.h"
#include "Coroutine.h"
#include "../thread/Thread.h"
class Scheduler
{
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

private:
    class CoroutineThread
    {
    public:
        CoroutineThread(Coroutine::ptr c, int thread): coroutine(c), threadId(thread){}
        CoroutineThread(Coroutine::ptr* c, int thread): coroutine(std::move(*c)), threadId(thread) {}
        CoroutineThread(std::function<void()> f, int thread): cb(f), threadId(thread) {} 
        CoroutineThread(std::function<void()>* f, int thread): cb(std::move(*f)), threadId(thread) {} 
        CoroutineThread(): threadId(-1) {}
        void reset()  
        {
            coroutine.reset();
            cb = nullptr;
            threadId = -1;
        }
        Coroutine::ptr coroutine;
        std::function<void()> cb;
        int threadId;
    };

public:
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    virtual ~Scheduler();

    inline const std::string& getName() const {return m_name;}

    static Scheduler* GetThis();
    static Coroutine* GetMainCoroutine();
    void start();
    void stop();

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

        if(emptyBefore)
            notify();
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

        if(emptyBefore)
            notify();
    }

    void switchTo(int thread = -1);
    std::ostream& dump(std::ostream& os);
protected:
    // notify schduler there are tasks
    virtual void notify();
    // main schedule function
    void run();
    virtual bool stopping();
    // no task and run this func
    virtual void idle();
    void setThis();
    inline bool hasIdleThreads() {  return m_idleTHreadCount > 0;}

private:

    template<class Task>
    bool scheduleNoLock(Task fc, int thread)
    {
        bool emptyBefore = m_coroutines.empty();
        CoroutineThread ct(fc, thread);
        if(ct.coroutine || ct.cb)
        {
            m_coroutines.push_back(ct);
        }

        return emptyBefore;
    }

    void runTask(CoroutineThread& ct);

private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::list<CoroutineThread> m_coroutines;    // coroutines ready to run
    Coroutine::ptr m_rootCoroutine;     // schedule coroutines when use_caller is true
    std::string m_name;  

protected:
    std::vector<int64_t> m_threadIds;
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activeThreadCount = {0};
    std::atomic<size_t> m_idleTHreadCount = {0};
    bool m_isStopping = true;
    bool m_autoStop = false;
    int64_t m_rootThreadId = 0;     // main thread(use_caller)
};

class SchedulerSwitcher: public Noncopyable
{
public:
    SchedulerSwitcher(std::shared_ptr<Scheduler> target);
    ~SchedulerSwitcher();
private:
    Scheduler* m_caller;
};
#endif