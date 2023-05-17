#ifndef THREAD_THREADLOOP_HPP
#define THREAD_THREADLOOP_HPP

#include <vector>
#include <atomic>
#include <map>
#include <memory>
#include <thread>
#include <functional>
#include "../log/Logger.h"

template<class T>
class ThreadPool
{
public:

    ThreadPool(int size = 4) : m_size(size), m_isQuited(false), m_threads(size) 
    {
        for(int i = 0; i < m_size; i++)
        {
            m_threads[i].reset(new T());
            m_idMap[m_threads[i]->id()] = m_threads[i];
        }
    }


    ThreadPool(std::function<void()> func, int size = 4) : ThreadPool(size)
    {

        for(int i = 0; i < m_size; i++)
        {
            m_threads[i]->bind(func);
        }
    }

    virtual ~ThreadPool()
    {
        if(m_isQuited)
            return;
        
        quit();
    }

    ThreadPool(const ThreadPool& pool) = delete;
    ThreadPool& operator =(const ThreadPool& rhs) = delete;

    virtual void start()
    {
        Logger::ptr logger = LOG_NAME("system");
        LOG_INFO(logger) << "thread poll start";
        m_isQuited = false;

        for(int i = 0; i < m_size; i++)
        {
            m_threads[i]->run();
        }
    }
    virtual void quit()
    {
        Logger::ptr logger = LOG_NAME("system");
        LOG_INFO(logger) << "thread poll start to quit";
        for(auto& thread: m_threads)
        {
            thread->quit();
        }

        m_isQuited = true;
        LOG_INFO(logger) << "thread poll quited";        
    }

    void detach()
    {
        for(auto& thread: m_threads)
            thread->detach();
    }

    void join()
    {
        for(auto& thread: m_threads)
            thread->join();        
    }

    std::shared_ptr<T> getNextThread() 
    {
        if(m_threads.empty())   return nullptr;

        static int no = 0;
        if(no >= m_threads.size())  
            no = 0;
        
        return m_threads[no];
    }

    std::shared_ptr<T> getThread(int no) 
    {
        if(no >= m_threads.size())   return nullptr;
        return m_threads[no];
    }
    
protected:
	int m_size;
    std::atomic_bool m_isQuited;
	std::vector<std::shared_ptr<T>> m_threads;
    std::map<std::thread::id, std::weak_ptr<T>> m_idMap;
};
#endif
