#ifndef NETWORK_EVENT_EVENTTHREADLOOP_H
#define NETWORK_EVENT_EVENTTHREADLOOP_H

#include <list>
#include <atomic>

#include "EventThread.h"
class EventThreadPool
{
public:
    EventThreadPool(int size = 4);
    ~EventThreadPool();

    EventThreadPool(const EventThreadPool& pool) = delete;
    EventThreadPool& operator =(const EventThreadPool& rhs) = delete;

    void start();
    void quit();
    
private:
    std::list<std::shared_ptr<EventThread>> m_threads;
    std::atomic_bool m_isQuited;
};
#endif