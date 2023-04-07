#include "log/Logger.h"

int main()
{
    LOG_ASSERT(false);
    LOG_ASSERT_W(1 == 0, "test log");

    return 0;
}