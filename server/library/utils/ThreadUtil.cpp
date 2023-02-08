#ifndef UTILS_THREAD_CPP
#define UTILS_THREAD_CPP

#include "ThreadUtil.h"

pid_t ThreadUtil::GetThreadId()
{
    return syscall(SYS_gettid);
}

#endif