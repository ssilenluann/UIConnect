#ifndef COROUTINE_COROUTINE_H
#define COROUTINE_COROUTINE_H

#include <ucontext.h>
#include <functional>
#include <memory>
#include <stdlib.h>

class Scheduler;

class Coroutine: public std::enable_shared_from_this<Coroutine>
{
public:
    typedef std::shared_ptr<Coroutine> ptr;

    // status of coroutine
    enum State
    {
        INIT,   // default
        HOLD,   // pause
        EXEC,   // in run
        TERM,   // end
        READY,  // ready
        EXCEPT  // exception
    };

private:
    // constructor of the first coroutine(main coroutine) for each thread
    Coroutine();
public:
    /**
     * @brief constructor
     * @param[in] cb main function of coroutine
     * @param[in] stack size
     * @param[in] scheduled by main coroutine
    */
    Coroutine(std::function<void()> cb, size_t stack_size = 0, bool use_caller = false);

    ~Coroutine();

    /**
     * @brief reset the main funciton of coroutine and the status
     * @pre state is INIT, TERM or EXCEPT
     * @post state will set to be INIT
    */
    void reset(std::function<void()> cb);
    
    /**
     * @brief switch from main coroutine to current coroutine, main coroutine value will be set into thread_local memory
    */
    void swapIn();

    /**
     * @brief switch from current coroutine to main coroutine, current coroutine value will be set into thread_local memory
    */
    void swapOut();

    inline uint64_t getId() const { return m_id;}
    inline State getState() const { return m_state;}
    inline void setState(State state) { m_state = state;}

public:

    /**
     * @brief seve target coroutine info into thread local memory
     * @param[in]  coroutine object pointor with context info
    */
    static void SetThreadCurrCoroutine(std::shared_ptr<Coroutine> t);

    /**
     * @brief get previous coroutine from thread local memory,
     *          if there isn't, then create a new Coroutine object from current context and save it into thread local memory
     * @return smart pointer of Coroutine object with saved context data
    */
    static Coroutine::ptr GetThreadCurrCoroutine();

    static Coroutine::ptr GetThreadRootCoroutine();

    /**
     * @brief create a main Coroutine onject with current context and then save it into thread local memory
    */
    static Coroutine::ptr Init();

    // swap out and set status to be READY
    static void Yield2Ready();

    // swap out and set status to be HOLD
    static void Yield2Hold();

    // count of coroutine
    static uint64_t CoroutineCount();

    // run the main function of coroutine, jump to main coroutine of thread after finish
    static void MainFunc();

    // run the main function of caller, jump to thread scheduler coroutine after finish
    static void CallerMainFunc();

    // running coroutine id
    static uint64_t CurrentCoroutineId();

private:
    uint16_t m_id = 0;  // coroutine id
    uint32_t m_stackSize = 0;   // stack size
    State m_state = INIT;   // state
    ucontext_t m_ctx;   // coroutine context
    void* m_stack = nullptr;    // stack point
    std::function<void()> m_cb; // main func of coroutine
};

class StackMemoryAllocator {
public:
    static void* Alloc(size_t size)
    {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size)
    {
        return free(vp);
    }
};
#endif