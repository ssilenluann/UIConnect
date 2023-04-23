#ifndef NETWORK_EVENT_EVENTTHREADLOOP_H
#define NETWORK_EVENT_EVENTTHREADLOOP_H

#include <vector>
#include <atomic>

#include "EventThread.h"
#include "../thread/ThreadPool.hpp"

class EventThreadPool: public ThreadPool<EventThread>
{
public:
    
    EventThreadPool(int size = 4): ThreadPool<EventThread>(size){}

    EventThreadPool(const EventThreadPool& pool) = delete;
    EventThreadPool& operator =(const EventThreadPool& rhs) = delete;

	std::shared_ptr<EventLoop> getNextLoop(); 
    inline std::shared_ptr<EventLoop> getLoop(std::thread::id& id)
    {
        return m_idMap[id].lock()->getLoop();
    }

};
#endif
