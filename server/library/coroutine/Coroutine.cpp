#ifndef COROUTINE_COROUTINE_CPP
#define COROUTINE_COROUTINE_CPP

#include <ucontext.h>
#include <memory>
class Coroutine: public std::enable_shared_from_this
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
     * @brief switch context from [main coroutine] to [this]
     * @pre state != EXEC
     * @post state will set to be EXEC 
    */
    void swapIn();

    /**
     * @brief switch context from [this] to [main coroutine]
    */
    void swapOut();

    /**
     * @brief save current context value into thread local memory, and set context of current thead to be [this]
    */
    void call();

    /**
     * @brief get old context value from thread local memory, save current context value into [this], 
     *        set current thread context as read from thread local memory
     * 
    */
    void back();

    inline uint64_t getId() const { return m_id;}
    inline State getState() const { return m_state;}

public:

    // set context value f into thread local memory
    static void SetThis(Coroutine* f);

    // get a smart pointer of current coroutine object
    static Coroutine::ptr GetThis();

    // swap out and set status to be READY
    static void Yield2Ready();

    // swap out and set status to be HOLD
    static void Yield2Hold();

    // count of coroutine
    static void CoroutineCount();

    // run the main function of coroutine, jump to main coroutine after finish
    static void MainFunc();

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
#endif