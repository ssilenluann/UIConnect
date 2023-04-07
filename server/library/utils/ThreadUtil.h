#ifndef UTILS_THREAD_H
#define UTILS_THREAD_H

#include <unistd.h>
#include <sys/syscall.h>
#include <vector>
#include <string>

class ThreadUtil
{
public:
    // get thread id
    static pid_t GetThreadId();

    /**
     * @brief get current call stacks
     * @param[out] bt call stacks
     * @param[in] size max stack size
     * @param[in] skip layers from stack top
     * */ 
    static void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 0);

    /**
     * @brief save call stacks as string
     * @param[in] size max stack size
     * @param[in] skip skip layers from stack top
     * @param[in] prefix prefix in the front of the stack string
    */
    static std::string Backtrace2String(int size = 64, int skip = 0, const std::string& prefix = "");

    static std::string demanle(const char* str);
};

#endif