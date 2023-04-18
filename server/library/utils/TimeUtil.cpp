#ifndef UTIL_TIMEUTIL_CPP
#define UTIL_TIMEUTIL_CPP

#include "TimeUtil.h"

int64_t TimeUtil::GetCurrentMicroSecondSinceEpoch()
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(now).count();
}

int64_t TimeUtil::GetCurrentMilliSecondSinceEpoch()
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

int64_t TimeUtil::GetCurrentSecondSinceEpoch()
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(now).count();
}

#endif