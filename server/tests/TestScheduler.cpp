#include "coroutine/Scheduler.h"
#include "utils/ThreadUtil.h"
#include "log/Logger.h"

#include <chrono>
#include <thread>
static Logger::ptr g_logger = LOG_ROOT();

void test_coroutine(std::shared_ptr<Scheduler<SchedulerWorker>> sc)
{
    static int s_count = 1;
    LOG_INFO(g_logger) << "test in coroutine, s_count = " << s_count << ", coroutine_id = " << ThreadUtil::GetCoroutineId();

    // sleep(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if(--s_count >= 0)
    {
        sc->schedule(std::bind(&test_coroutine, sc), ThreadUtil::GetThreadId());
    }
}

int main()
{
    LOG_INFO(g_logger) << "main";
    std::shared_ptr<Scheduler<SchedulerWorker>> sc = std::make_shared<Scheduler<SchedulerWorker>>(1, "test");
    sc->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    LOG_INFO(g_logger) << "schedule";
    sc->schedule(std::bind(&test_coroutine, sc));
    sc->stop();
    LOG_INFO(g_logger) << "over";
    return 0;

}