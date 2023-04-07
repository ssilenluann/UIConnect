#include "thread/Thread.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <threads.h> 
#include <unistd.h>
#include <sys/syscall.h>  
int main()
{

    std::thread::id threadId = std::this_thread::get_id();
    size_t hash = std::hash<std::thread::id>()(threadId);
    unsigned int id = static_cast<unsigned int>(hash);
    std::cout << "Thread ID as int: " << id << std::endl;

    std::cout << threadId << std::endl;
    std::cout << syscall(SYS_gettid) << std::endl;

    return 0;

}