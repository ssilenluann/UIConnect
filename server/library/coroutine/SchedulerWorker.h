#ifndef COROUTINE_SCHEDULERWORKER_H
#define COROUTINE_SCHEDULERWORKER_H

#include "../thread/Thread.h"
#include "../coroutine/Coroutine.h"

#include <memory>

template <class T>
class Scheduler;

class SchedulerWorker: public Thread, public std::enable_shared_from_this<SchedulerWorker>
{
public:
    typedef std::shared_ptr<SchedulerWorker> ptr;

    SchedulerWorker(const std::string &name = ""): Thread(){}

    void setScheduler(std::shared_ptr<Scheduler<SchedulerWorker>> sc);

    virtual void work();
    virtual bool readyToQuit();
    virtual void bind(std::function<void(std::shared_ptr<SchedulerWorker>)> funcWithThread);
    void entry() override;

protected:
    std::weak_ptr<Scheduler<SchedulerWorker>> m_scheduler;
    std::function<void(std::shared_ptr<SchedulerWorker>)> m_schedulerFunc;
};

#endif