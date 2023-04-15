#ifndef COROUTINE_COROUTINEWORKTHREAD_H
#define COROUTINE_COROUTINEWORKTHREAD_H

#include "Coroutine.h"
#include "../thread/Thread.h"
#include "../thread/ThreadPool.hpp"
#include "../Mutex.h"

#include <memory>
#include <string>

class Scheduler;
struct ThreadJobTarget;
class CoroutineWorkThread: public Thread
{
public:
    typedef std::shared_ptr<CoroutineWorkThread> ptr;
    typedef Mutex MutexType;
    CoroutineWorkThread(){};
    CoroutineWorkThread(std::shared_ptr<Scheduler> scheduler, const std::string& name = "");
    ~CoroutineWorkThread();

    void run();

    /**
     * @brief main func of thread, get coroutine from scheduler and run it
    */
    void runCoroutine();

    void runCoroutineTask(ThreadJobTarget& job);

    /**
     * @brief idle, called by idle coroutine, 
     *      when no task of this thread, this thread will switch to idle coroutine to run this func
    */
   virtual void idle();
   static std::shared_ptr<Scheduler> GetScheduler();

private:
    std::weak_ptr<Scheduler> m_scheduler;
    std::shared_ptr<Coroutine> m_idleCoroutine;

};

// thread work class, coule be assigned to a thread, then thread will call it's function or coroutine
struct ThreadJobTarget
{
    ThreadJobTarget(Coroutine::ptr c, int thread): coroutine(c), workThreadId(thread){}
    ThreadJobTarget(std::function<void()> f, int thread): cb(f), workThreadId(thread) {} 
    ThreadJobTarget(): workThreadId(-1) {}
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

class CoroutineWorkThreadPool: public ThreadPool<CoroutineWorkThread>
{
public:
    CoroutineWorkThreadPool(int threadCount): ThreadPool(threadCount) {}

    virtual void start(std::shared_ptr<Scheduler> scheduler, const std::string& name = "")
    {
        Logger::ptr logger = LOG_NAME("system");
        LOG_INFO(logger) << "thread poll start";
        m_isQuited = false;

        for(int i = 0; i < m_size; i++)
        {
            m_threads[i].reset(new CoroutineWorkThread(scheduler, name + "_" + std::to_string(i)));
            m_idMap[m_threads[i]->id()] = m_threads[i];
            m_threads[i]->run();
        }
    }

};

#endif