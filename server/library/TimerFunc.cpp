
#include <memory>
#include <vector>
#include <set>
#include "thread/Thread.h"

#include "TimerFunc.h"
#include "utils/TimeUtil.h"

TimerFunc::TimerFunc(uint64_t ms, std::function<void()> cb, bool oneshot, std::weak_ptr<TimerManager> manager)
    : m_oneshot(oneshot), m_cycle(ms), m_cb(cb), m_manager(manager)
{
    m_nextCallTime = TimeUtil::GetCurrentMilliSecondSinceEpoch() + ms;
}

bool TimerFunc::cancel()
{
    auto manager = m_manager.lock();
    TimerManager::RWMutexType::WriteLock lock(manager->m_mutex);
    if(m_cb)
    {
        m_cb = nullptr;
        auto it = manager->m_timers.find(shared_from_this());
        if(it != manager->m_timers.end())
            manager->m_timers.erase(it);
        return true;
    }

    return false;
}

bool TimerFunc::refresh()
{
    auto manager = m_manager.lock();
    TimerManager::RWMutexType::WriteLock lock(manager->m_mutex);
    if(!m_cb)
        return false;
    
    auto it = manager->m_timers.find(shared_from_this());
    if(it == manager->m_timers.end())
        return false;

    manager->m_timers.erase(it);
    m_nextCallTime = TimeUtil::GetCurrentMilliSecondSinceEpoch() + m_cycle;
    manager->m_timers.insert(shared_from_this());
    return true;
}

bool TimerFunc::reset(uint64_t ms, bool from_now)
{
    auto manager = m_manager.lock();
    if(m_cycle == ms && !from_now)
        return true;
    
    TimerManager::RWMutexType::WriteLock lock(manager->m_mutex);
    if(!m_cb)   return false;

    auto it = manager->m_timers.find(shared_from_this());
    if(it == manager->m_timers.end())
        return false;
    manager->m_timers.erase(it);

    uint64_t start = from_now? TimeUtil::GetCurrentMilliSecondSinceEpoch()
                        : m_nextCallTime - m_cycle;
    
    m_cycle = ms;
    m_nextCallTime = start + m_cycle;
    manager->queue(shared_from_this());
    return false;
}

bool TimerFunc::TimeConparator::operator()(const TimerFunc::ptr &lhs, const TimerFunc::ptr &rhs) const
{
    if(!lhs && !rhs)    return false;
    if(!lhs)    return true;
    if(!rhs)    return false;
    
    if(lhs->m_nextCallTime < rhs->m_nextCallTime)
        return true;
    if(rhs->m_nextCallTime < lhs->m_nextCallTime)
        return false;
    
    return lhs.get() < rhs.get();
}


TimerManager::TimerManager()
{
    m_previousTime = TimeUtil::GetCurrentMilliSecondSinceEpoch();
}

TimerManager::~TimerManager() {}

TimerFunc::ptr TimerManager::addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot)
{
    TimerFunc::ptr timer(new TimerFunc(cycle, cb, oneshot, shared_from_this()));
    RWMutexType::WriteLock lock(m_mutex);
    queue(timer);
    return timer;
}

TimerFunc::ptr TimerManager::addConditionTimer(uint64_t cycle, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool oneshot)
{
    return addTimer(cycle, std::bind(&TimerManager::OnConditionTimer, weak_cond, cb), oneshot);
}

uint64_t TimerManager::getNextTimer()
{
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled = false;
    if(m_timers.empty())
        return 0;
    
    auto& next = *m_timers.begin();
    uint64_t cur = TimeUtil::GetCurrentMilliSecondSinceEpoch();
    return cur >= next->m_nextCallTime ? 0 : next->m_nextCallTime - cur;
}

bool TimerManager::hasTimer()
{
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}

void TimerManager::OnConditionTimer(std::weak_ptr<void> cond, std::function<void()> cb)
{
    std::shared_ptr<void> tmp = cond.lock();
    if(tmp) cb();
}

void TimerManager::setOnTimerInsertedAtFront(std::function<void()> cb)
{
    m_onFrontInsertedCallback = cb;
}

void TimerManager::queue(TimerFunc::ptr timer)
{
    auto it = m_timers.insert(timer).first;
    bool atFront = (it == m_timers.begin()) && !m_tickled;
    if(atFront && m_onFrontInsertedCallback)
        m_onFrontInsertedCallback();
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs)
{
    auto cur = TimeUtil::GetCurrentMilliSecondSinceEpoch();
    std::vector<TimerFunc::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty())    return;
    }

    RWMutexType::WriteLock lock(m_mutex);
    if(m_timers.empty())    return;

    auto it = m_timers.begin();
    while(it != m_timers.end() && (*it)->m_nextCallTime <= cur) 
    {
        expired.push_back(*it);
        it = m_timers.erase(it);
    }
    
    for(auto& timer: expired)
    {
        cbs.push_back(timer->m_cb);
        if(!timer->m_oneshot)
        {
            timer->m_nextCallTime = cur + timer->m_cycle;
            m_timers.insert(timer);
        }
        else
        {
            timer->m_cb = nullptr;
        }
    }

}
