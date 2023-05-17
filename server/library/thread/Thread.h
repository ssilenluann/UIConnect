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
    typedef std::shared_ptr<Thread> ptr;

    Thread(const std::string& name = "");
    virtual ~Thread();

    Thread(const Thread& thread) = delete;
    Thread& operator =(const Thread& rhs) = delete;

    virtual void run();
    virtual void quit();
    inline std::thread::id id() { return m_threadId;}
    inline pid_t pid() { return m_pid;}
    inline std::string name() { return m_name;}

    void bind(std::function<void()> func);
    virtual void entry();

    static const std::string& GetName();
    static void SetName(const std::string& name);
    void detach();
    void join();
protected:
	std::thread::id m_threadId;
    pid_t m_pid;
    std::string m_name;
    std::shared_ptr<std::thread> m_thread;
    std::function<void()> m_func;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic_bool m_isQuited;    // quit func has been called
    std::atomic_bool m_isEnd;       // thread entry func ended
};
#endif