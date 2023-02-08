#ifndef THREAD_THREAD_H
#define THREAD_THREAD_H

#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <condition_variable>

class Thread
{

public:
    Thread();
    virtual ~Thread();

    Thread(const Thread& thread) = delete;
    Thread& operator =(const Thread& rhs) = delete;

    virtual void run();
    virtual void quit();
    inline std::thread::id id()
    {
        return m_threadId;
    }
    void bind(std::function<void()> func);
    void entry();
    
protected:
	std::thread::id m_threadId;
    std::shared_ptr<std::thread> m_thread;
    std::function<void()> m_func;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic_bool m_isQuited;    // quit func has been called
    std::atomic_bool m_isEnd;       // thread entry func ended
};
#endif