#ifndef UTILS_TIMEUITL_H
#define UTILS_TIMEUITL_H

#include <chrono>
class TimeUtil
{
public:

    static int64_t GetCurrentMicroSecondSinceEpoch();
    static int64_t GetCurrentMilliSecondSinceEpoch();
    static int64_t GetCurrentSecondSinceEpoch();
};
#endif