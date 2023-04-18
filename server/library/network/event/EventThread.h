#ifndef NETWORK_EVENT_EVENTTHREAD_H
#define NETWORK_EVENT_EVENTTHREAD_H

#include <thread>
#include <atomic>
#include "EventLoop.h"
#include "../thread/Thread.h"
class EventThread: public Thread
{

public:
    EventThread();
    ~EventThread();

    EventThread(const EventThread& thread) = delete;
    EventThread& operator =(const EventThread& rhs) = delete;

    void run();
    void quit();
	std::shared_ptr<EventLoop> getLoop();
    inline std::thread::id id()
    {
        return m_threadId;
    }
    
private:
    std::shared_ptr<EventLoop> m_loop;

};
#endif
