#ifndef NETWORK_EVENT_EVENTTHREADLOOP_H
#define NETWORK_EVENT_EVENTTHREADLOOP_H

#include <vector>
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
	std::shared_ptr<EventLoop> getNextLoop(); 
    inline std::shared_ptr<EventLoop> getLoop(std::thread::id& id)
    {
        return m_idMap[id].lock()->getLoop();
    }
    
private:	
	int m_size;
    std::atomic_bool m_isQuited;
	std::vector<std::shared_ptr<EventThread>> m_threads;
    std::map<std::thread::id, std::weak_ptr<EventThread>> m_idMap;
};
#endif
