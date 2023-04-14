#include "coroutine/Coroutine.h"
#include "coroutine/Scheduler.h"
#include "utils/ThreadUtil.h"
#include "log/Logger.h"

#include <chrono>
#include <thread>
static Logger::ptr g_logger = LOG_ROOT();

void test_coroutine()
{
    static int s_count = 5;
    LOG_INFO(g_logger) << "test in coroutine, s_count = " << s_count;

    // sleep(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if(--s_count >= 0)
        Scheduler::GetThis()->schedule(&test_coroutine, ThreadUtil::GetThreadId());
}

int main()
{
    LOG_INFO(g_logger) << "main";
    Scheduler sc(3, false, "test");
    sc.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_coroutine);
    sc.stop();
    LOG_INFO(g_logger) << "over";
    return 0;

}