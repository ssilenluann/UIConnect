#include "log/Logger.h"
static Logger::ptr g_logger = LOG_ROOT();
int main()
{
    LOG_ASSERT(false);
    LOG_ASSERT_W(1 == 0, "test log");
    LOG_INFO(g_logger) << "run";
    return 0;
}