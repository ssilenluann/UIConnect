#ifndef NETWORK_EVENT_EVENTTHREAD_H
#define NETWORK_EVENT_EVENTTHREAD_H

#include <thread>
#include <atomic>
#include "EventLoop.h"
class EventThread
{

public:
    EventThread();
    ~EventThread();

    EventThread(const EventThread& thread) = delete;
    EventThread& operator =(const EventThread& rhs) = delete;

    void run();
    void quit();
	std::shared_ptr<EventLoop> getLoop();
private:
    std::shared_ptr<std::thread> m_thread;
    std::shared_ptr<EventLoop> m_loop;
    
	std::thread::id m_threadId;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic_bool m_isQuited;
};
#endif
