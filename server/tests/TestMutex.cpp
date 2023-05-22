#include "Mutex.h"
#include "log/Logger.h"
#include <thread>
#include <functional>
#include <string.h>
// static Logger::ptr g_logger = LOG_ROOT();

RWMutex rwMutex;

void thread_func_wrlock()
{
    rwMutex.wrlock();
    std::cout << "wrlocked" << std::endl;
    for(;;);
}

void thread_func_rdlock()
{
    rwMutex.rdlock();
    std::cout << "rdlocked" << std::endl;
    for(;;);
}

int main()
{
    

    // // LOG_INFO(g_logger) << "create rwmutex";
    // RWMutex::ReadLock rlock(rwMutex);

    // // LOG_INFO(g_logger) << "unlock rwmutex";
    // rlock.unlock();

    // rwMutex.wrlock();
    // rwMutex.rdlock();
    // rlock.unlock();
    // pthread_rwlock_t m_lock;
    // pthread_rwlock_init(&m_lock, nullptr);

    // std::cout << ThreadUtil::GetThreadId() << ", cor id: " << ThreadUtil::GetCoroutineId() << " rdlock: " << (uint64_t)&m_lock 
    //     << ", holding read lock threads count = " << m_lock.__data.__readers << ", waitting writers no = " << m_lock.__data.__writers 
    //     << ", current writer thread no = " << m_lock.__data.__cur_writer << std::endl;  

    // pthread_rwlock_rdlock(&m_lock);
    // pthread_rwlock_rdlock(&m_lock);
    // pthread_rwlock_rdlock(&m_lock);

    // std::cout << ThreadUtil::GetThreadId() << ", cor id: " << ThreadUtil::GetCoroutineId() << " rdlock: " << (uint64_t)&m_lock 
    //     << ", holding read lock threads count = " << m_lock.__data.__readers << ", waitting writers no = " << m_lock.__data.__writers 
    //     << ", current writer thread no = " << m_lock.__data.__cur_writer << std::endl;  

    // pthread_rwlock_unlock(&m_lock);
    // pthread_rwlock_unlock(&m_lock);
    // pthread_rwlock_unlock(&m_lock);

    // int retp = 0;
    // retp = pthread_rwlock_wrlock(&m_lock);
    // retp = pthread_rwlock_rdlock(&m_lock);
    // if(retp != 0)
    // {
    //     std::cout << strerror(retp) << std::endl;
    // }
    // pthread_rwlock_unlock(&m_lock);

    
    // // pthread_rwlock_rdlock(&m_lock);
    // // retp = pthread_rwlock_wrlock(&m_lock);
    // // if(retp != 0)
    // // {
    // //     std::cout << strerror(retp) << std::endl;
    // // }
    
    // pthread_rwlock_wrlock(&m_lock);
    // pthread_rwlock_unlock(&m_lock);

    // pthread_rwlock_wrlock(&m_lock);
    // pthread_rwlock_unlock(&m_lock);
    
    // pthread_rwlock_destroy(&m_lock);

    // std::thread t1(thread_func_wrlock);
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // std::thread t2(thread_func_rdlock);
    std::thread t1(thread_func_rdlock);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::thread t2(thread_func_wrlock);
    for(;;);
    return 0;
}