#ifndef THREAD_THREADLOOP_HPP
#define THREAD_THREADLOOP_HPP

#include <vector>
#include <atomic>
#include <map>
#include <memory>
#include <thread>
#include "../log/Logger.h"

template<class T>
class ThreadPool
{
public:
    ThreadPool(int size = 4): m_size(size), m_isQuited(false), m_threads(size){}
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
            m_threads[i].reset(new T());
            m_idMap[m_threads[i]->id()] = m_threads[i];
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
    
public:
	int m_size;
    std::atomic_bool m_isQuited;
	std::vector<std::shared_ptr<T>> m_threads;
    std::map<std::thread::id, std::weak_ptr<T>> m_idMap;
};
#endif
