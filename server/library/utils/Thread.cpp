#ifndef UTILS_THREAD_CPP
#define UTILS_THREAD_CPP

#include "Thread.h"

pid_t Thread::GetThreadId()
{
    return syscall(SYS_gettid);
}

#endif