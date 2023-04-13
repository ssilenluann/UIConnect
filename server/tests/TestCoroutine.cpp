#include "coroutine/Coroutine.h"
#include "log/Logger.h"
// #include "utils/ThreadUtil.h"

static Logger::ptr g_logger = LOG_ROOT();

void run()
{
    LOG_INFO(g_logger) << "run in coroutine";
    LOG_INFO(g_logger) << "swap out";
    Coroutine::Yield2Hold();
    LOG_INFO(g_logger) << "coroutine func end";
    LOG_INFO(g_logger) << "swap out";
    Coroutine::Yield2Hold();
}

void test()
{
    LOG_INFO(g_logger) << "main coroutine";
    Coroutine::Init();
    LOG_INFO(g_logger) << "main begin";

    Coroutine::ptr coroutine(new Coroutine(run));
    coroutine->call();
    LOG_INFO(g_logger) << "main after call";

    coroutine->call();
    LOG_INFO(g_logger) << "main after end";

    coroutine->call();
    LOG_INFO(g_logger) << "test end";
}

int main() 
{
    test();
    return 0;
}