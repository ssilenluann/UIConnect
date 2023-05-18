#ifndef COROUTINE_SCHEDULERWORKER_CPP
#define COROUTINE_SCHEDULERWORKER_CPP

#include "SchedulerWorker.h"
#include "Scheduler.h"

void SchedulerWorker::setScheduler(std::shared_ptr<Scheduler<SchedulerWorker>> sc)
{
    m_scheduler = sc;
}

void SchedulerWorker::work()
{

    // LOG_INFO(g_logger) << "idle";
    while(!readyToQuit())
        Coroutine::Yield2Hold();

}

bool SchedulerWorker::readyToQuit()
{
    if(m_scheduler.expired())   return true;
    return m_scheduler.lock()->isStopping() && m_scheduler.lock()->allowWorkerQuit();        
}

void SchedulerWorker::bind(std::function<void(std::shared_ptr<SchedulerWorker>)> funcWithThread)
{
    m_schedulerFunc = funcWithThread;
}

void SchedulerWorker::entry()
{
    // LOG_INFO(g_logger) << "thread entry start, thread_name = " << m_name;
    if(!m_schedulerFunc)
    {
        // LOG_ERROR(g_logger) << "thread main func unvalid, thread entry exit";
        return;
    }

    m_schedulerFunc(shared_from_this());
    // LOG_ERROR(g_logger) << "thread entry function end, thread_name = " << m_name;
    
    m_isEnd = true;
    m_cv.notify_all();
}
#endif
