#ifndef COROUTINE_COROUTINE_CPP
#define COROUTINE_COROUTINE_CPP

#include <atomic>
#include <memory>
#include "Coroutine.h"
#include "Scheduler.h"
#include "../log/Logger.h"
#include "../log/LogConfItem.h"
#include "../config/Config.h"
#include "../utils/ThreadUtil.h"

static Logger::ptr g_logger = LOG_NAME("system");

static std::atomic<uint64_t> s_coroutine_id = {0};
static std::atomic<uint64_t> s_coroutine_count = {0};

static thread_local std::weak_ptr<Coroutine> t_thread_temp_coroutine;
static thread_local Coroutine::ptr t_thread_main_coroutine = {0};

static ConfigItem<uint32_t>::ptr g_coroutine_stack_size = 
    Config::SearchOrAdd<uint32_t>("coroutine.stack_size", 128*1024, "coroutine stack size");

#endif

Coroutine::Coroutine()
{
    m_state = EXEC;

    if(getcontext(&m_ctx))
    {
        LOG_ERROR(g_logger) << "getcontext failed: " << strerror(errno);
        LOG_ASSERT_W(false, "getcontext");
    }

    ++s_coroutine_count;
    LOG_DEBUG(g_logger) << "main coroutine created";
}

Coroutine::Coroutine(std::function<void()> cb, size_t stack_size, bool use_caller) : m_id(++s_coroutine_id), m_cb(cb)
{
    ++s_coroutine_count;
    m_stackSize = stack_size > 0 ? stack_size : g_coroutine_stack_size->getValue();
    m_stack = StackMemoryAllocator::Alloc(m_stackSize);

    if(getcontext(&m_ctx))
    {
        LOG_ERROR(g_logger) << "getcontext failed: " << strerror(errno);
        LOG_ASSERT_W(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stackSize;

    makecontext(
        &m_ctx,
        use_caller? &Coroutine::CallerMainFunc : &Coroutine::MainFunc,
        0
    );

    LOG_INFO(g_logger) << "create coroutine, id = " << m_id;

}

Coroutine::~Coroutine()
{
    --s_coroutine_count;
    if(m_stack)
    {
        LOG_ASSERT(
            m_state == TERM || m_state == EXCEPT || m_state == INIT
        );
    }
    else
    {
        LOG_ASSERT(!m_cb);
        LOG_ASSERT(m_state == EXEC);

        if(t_thread_temp_coroutine.lock().get() == this)
            t_thread_temp_coroutine.reset();
    }
    
    LOG_DEBUG(g_logger) << "Coroutine::~Coroutine(), id = " << m_id << ", total = " << s_coroutine_count;
}

void Coroutine::reset(std::function<void()> cb)
{
    LOG_ASSERT(m_stack);;
    LOG_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);

    m_cb = cb;
    if(getcontext(&m_ctx))
    {
        LOG_ERROR(g_logger) << "getcontext failed: " << strerror(errno);
        LOG_ASSERT_W(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stackSize;

    makecontext(&m_ctx, &Coroutine::MainFunc, 0);
    m_state = INIT;
}

void Coroutine::swapIn()
{
    SaveTemp(shared_from_this());
    LOG_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainCoroutine()->m_ctx, &m_ctx))
    {
        LOG_ERROR(g_logger) << "swapcontext failed: " << strerror(errno);
        LOG_ASSERT_W(false, "swapcontext");
    }
    
}

void Coroutine::swapOut()
{
    SaveTemp(Scheduler::GetMainCoroutine()->shared_from_this());
    if(swapcontext(&m_ctx, &Scheduler::GetMainCoroutine()->m_ctx))
    {
        LOG_ERROR(g_logger) << "swapcontext failed: " << strerror(errno);
        LOG_ASSERT_W(false, "swapcontext");
    }
}

void Coroutine::call()
{
    SaveTemp(shared_from_this());
    m_state = EXEC;
    if(swapcontext(&t_thread_main_coroutine->m_ctx, &m_ctx))
    {
        LOG_ERROR(g_logger) << "swapcontext failed: " << strerror(errno);
        LOG_ASSERT_W(false, "swapcontext");
    }
}

void Coroutine::back()
{
    SaveTemp(t_thread_main_coroutine);
    if(swapcontext(&m_ctx, &t_thread_main_coroutine->m_ctx))
    {
        LOG_ERROR(g_logger) << "swapcontext failed: " << strerror(errno);
        LOG_ASSERT_W(false, "swapcontext");
    }
}

void Coroutine::SaveTemp(std::shared_ptr<Coroutine> t)
{
    t_thread_temp_coroutine = t;
}

Coroutine::ptr Coroutine::GetTemp()
{
    if(t_thread_temp_coroutine.lock().get() == nullptr)
        Init();

    return t_thread_temp_coroutine.lock();
}

Coroutine::ptr Coroutine::Init()
{
    if(t_thread_temp_coroutine.lock().get() != nullptr)
    {
        LOG_ASSERT_W(false, "main coroutine object for current has been created");
        return t_thread_temp_coroutine.lock();
    }

    t_thread_main_coroutine = std::shared_ptr<Coroutine>(new Coroutine());
    t_thread_temp_coroutine = t_thread_main_coroutine;
    return t_thread_temp_coroutine.lock();
}

void Coroutine::Yield2Ready()
{
    Coroutine::ptr cur = GetTemp();
    LOG_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

void Coroutine::Yield2Hold()
{
    Coroutine::ptr cur = GetTemp();
    LOG_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;
    cur->swapOut();
}

uint64_t Coroutine::CoroutineCount()
{
    return s_coroutine_count;
}

void Coroutine::MainFunc()
{
    Coroutine::ptr cur = GetTemp();
    LOG_ASSERT(cur);

    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }
    catch(const std::exception& e)
    {
        cur->m_state = EXCEPT;
        LOG_ERROR(g_logger) << "Coroutine Except: " << e.what()
            << ", fiber_id = " << cur->getId()
            << std::endl
            << ThreadUtil::Backtrace2String();
    }
    catch(...)
    {
        cur->m_state = EXCEPT;
        LOG_ERROR(g_logger) << "Coroutine Except, "
            << ", fiber_id = " << cur->getId()
            << std::endl
            << ThreadUtil::Backtrace2String();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();
    
    LOG_ASSERT_W(false, "coroutine should have been swithed out before");
}

void Coroutine::CallerMainFunc()
{
    Coroutine::ptr cur = GetTemp();
    LOG_ASSERT(cur);

    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }
    catch(const std::exception& e)
    {
        cur->m_state = EXCEPT;
        LOG_ERROR(g_logger) << "Coroutine Except: " << e.what()
            << ", fiber_id = " << cur->getId()
            << std::endl
            << ThreadUtil::Backtrace2String();
    }
    catch(...)
    {
        cur->m_state = EXCEPT;
        LOG_ERROR(g_logger) << "Coroutine Except, "
            << ", fiber_id = " << cur->getId()
            << std::endl
            << ThreadUtil::Backtrace2String();
    }
    
    // here, if we don't get the raw pointer and then reset the shared_ptr object 'cur',
    // before the 'cur' destoried, the context has been switched out,
    // so the use_count won't decrese, and lead to memory leak
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    LOG_ASSERT_W(false, "coroutine should have been swithed out before");
}

uint64_t Coroutine::CurrentCoroutineId()
{
    if(t_thread_main_coroutine) {
        return t_thread_main_coroutine->getId();
    }
    return 0;
}
