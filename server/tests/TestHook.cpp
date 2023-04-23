#include "hook/Hook.h"
#include "log/Logger.h"
#include "config/Config.h"
#include "coroutine/IOManager.h"

static Logger::ptr g_logger = LOG_ROOT();

void test_sleep(const IOManager::ptr& iom)
{
    iom->schedule([]()
    {
        sleep(2);
        LOG_INFO(g_logger) << "sleep 2";
    });

    iom->schedule([]()
    {
        sleep(3);
        LOG_INFO(g_logger) << "sleep 3";
    }); 

    LOG_INFO(g_logger) << "test sleep";
}

int main(int argc, char** argv)
{
    IOManager::ptr iom = std::shared_ptr<IOManager>(new IOManager(1, "test hook"));
    iom->start(iom);

    test_sleep(iom);
    return 0;
}