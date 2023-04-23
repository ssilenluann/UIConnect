#ifndef MUTEX_H
#define MUTEX_H

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>
#include <iostream>
#include "Noncopiable.h"
#include "utils/ThreadUtil.h"

// linux semaphore wrap class
class Semaphore : Noncopyable 
{
public:
	/*
	* @param[in] count: init value
	**/
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
	void notify();
	
private:
    sem_t m_semaphore;
};

// lock guard wrap class
template<class T>
struct ScopedLockImpl 
{
public:

    ScopedLockImpl(T& mutex)
    :m_mutex(mutex) 
    {
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl() 
    {
        unlock();
    }

    void lock() 
    {
        if(!m_locked) 
        {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() 
    {
        if(m_locked) 
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    // mutex
    T& m_mutex;
    // is locked
    bool m_locked;
};

// lock guard for read lock
template<class T>
struct ReadScopedLockImpl {
public:

    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }


    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    // mutex
    T& m_mutex;
    // is locked
    bool m_locked;
};

// lock guard for write lock
template<class T>
struct WriteScopedLockImpl {
public:

    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    // Mutex
    T& m_mutex;
    // is locked
    bool m_locked;
};

// linux mutex wrap class
class Mutex : Noncopyable {
public: 
    typedef ScopedLockImpl<Mutex> Lock;

    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        // std::cout << ThreadUtil::Backtrace2String();
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    /// mutex
    pthread_mutex_t m_mutex;
};

// null mutex, designed for debug
class NullMutex : Noncopyable{
public:
    typedef ScopedLockImpl<NullMutex> Lock;

    NullMutex() {}
    ~NullMutex() {}

    void lock() {}
    void unlock() {}
};

// read/write mutex wrap class
class RWMutex : Noncopyable{
public:

    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    
    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};

// null RWMutex wrap class
class NullRWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    typedef WriteScopedLockImpl<NullMutex> WriteLock;

    NullRWMutex() {}
    ~NullRWMutex() {}

    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};

// linux spinlock wrap class
class Spinlock : Noncopyable {
public:
    typedef ScopedLockImpl<Spinlock> Lock;

    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }
    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    
    pthread_spinlock_t m_mutex;
};

// CAS Mutex
class CASLock : Noncopyable {
public:
    typedef ScopedLockImpl<CASLock> Lock;

    CASLock() {
        m_mutex.clear();
    }

    ~CASLock() {
    }

    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }

    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:

    volatile std::atomic_flag m_mutex;
};

// class Scheduler;
// class FiberSemaphore : Noncopyable {
// public:
    // typedef Spinlock MutexType;

    // FiberSemaphore(size_t initial_concurrency = 0);
    // ~FiberSemaphore();

    // bool tryWait();
    // void wait();
    // void notify();

    // size_t getConcurrency() const { return m_concurrency;}
    // void reset() { m_concurrency = 0;}
// private:
    // MutexType m_mutex;
    // std::list<std::pair<Scheduler*, Fiber::ptr> > m_waiters;
    // size_t m_concurrency;
// };

#endif