#ifndef UTILS_THREADUTIL_CPP
#define UTILS_THREADUTIL_CPP
#include <execinfo.h>

#include "../log/Logger.h"

#include "ThreadUtil.h"
#include "StringUtil.h"

#include <unistd.h>
#include <sys/syscall.h>


pid_t ThreadUtil::GetThreadId()
{
    return syscall(SYS_gettid);
}

void ThreadUtil::Backtrace(std::vector<std::string> &bt, int size, int skip)
{
    void** array = (void**)malloc(sizeof(void*) * size);
    size_t s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if(strings == nullptr)
    {
        free(strings);
        free(array);
        LOG_ERROR(LOG_NAME("system")) << "backtrace_symbols error";
        return;
    }

    for(size_t i = skip; i < s; i++)
    {
        bt.push_back(demanle(strings[i]));
    }

}

std::string ThreadUtil::Backtrace2String(int size, int skip, const std::string & prefix)
{
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    
    std::stringstream ss;
    for(auto& stack: bt)
    {
        ss << prefix << stack << std::endl;
    }

    return ss.str();
}


std::string ThreadUtil::demanle(const char * str)
{
    size_t size = 0; 
    int status = 0;
    std::string rt;
    rt.resize(256);

    if(sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0]) == 1)
    {
        char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
        if(v)
        {
            std::string result(v);
            free(v);
            return result;
        }
    }
    if(sscanf(str, "%255s", &rt[0]))
    {
        return rt;
    }

    return str;
}

uint32_t ThreadUtil::GetCoroutineId()
{
    return 0;
}

#endif