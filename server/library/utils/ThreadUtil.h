#ifndef UTILS_THREAD_H
#define UTILS_THREAD_H

#include <unistd.h>
#include <sys/syscall.h>

class ThreadUtil
{
public:
    static pid_t GetThreadId();
};

#endif