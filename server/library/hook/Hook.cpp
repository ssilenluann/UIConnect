#ifndef HOOK_HOOK_CPP
#define HOOK_HOOK_CPP

#include "Hook.h"
#include "log/Logger.h"
#include "config/Config.h"
#include "FDItem.h"
#include "FDManager.h"
#include "TimerFunc.h"
#include "../network/epoll/EpollScheduler.h"

static thread_local bool t_hook_enabled = false;
static Logger::ptr g_logger = LOG_NAME("system");
static ConfigItem<int>::ptr g_tcp_connect_timeout = Config::SearchOrAdd("tcp.connect.timeout", 5000, "tcp connect timeout");

bool Hook::IsHookEnabled()
{
    return t_hook_enabled;
}

void Hook::SetHookEnable(bool flag)
{
    t_hook_enabled = flag;
}

#define HOOK_FUNC_DEF(name) name ## _fun  name ## _f 
#define HOOK_FUNC_INI(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name)

HOOK_FUNC_DEF(sleep);
HOOK_FUNC_DEF(usleep);
HOOK_FUNC_DEF(nanosleep);
HOOK_FUNC_DEF(socket);
HOOK_FUNC_DEF(connect);
HOOK_FUNC_DEF(accept);
HOOK_FUNC_DEF(read);
HOOK_FUNC_DEF(readv);
HOOK_FUNC_DEF(recv);
HOOK_FUNC_DEF(recvfrom);
HOOK_FUNC_DEF(recvmsg);
HOOK_FUNC_DEF(write);
HOOK_FUNC_DEF(writev);
HOOK_FUNC_DEF(send);
HOOK_FUNC_DEF(sendto);
HOOK_FUNC_DEF(sendmsg);
HOOK_FUNC_DEF(close);
HOOK_FUNC_DEF(fcntl);
HOOK_FUNC_DEF(ioctl);
HOOK_FUNC_DEF(getsockopt);
HOOK_FUNC_DEF(setsockopt);

void hook_init()
{
    static bool is_inited = false;
    if(is_inited)   return;

    is_inited = true;

    HOOK_FUNC_INI(sleep);
    HOOK_FUNC_INI(usleep);
    HOOK_FUNC_INI(nanosleep);
    HOOK_FUNC_INI(socket);
    HOOK_FUNC_INI(connect);
    HOOK_FUNC_INI(accept);
    HOOK_FUNC_INI(read);
    HOOK_FUNC_INI(readv);
    HOOK_FUNC_INI(recv);
    HOOK_FUNC_INI(recvfrom);
    HOOK_FUNC_INI(recvmsg);
    HOOK_FUNC_INI(write);
    HOOK_FUNC_INI(writev);
    HOOK_FUNC_INI(send);
    HOOK_FUNC_INI(sendto);
    HOOK_FUNC_INI(sendmsg);
    HOOK_FUNC_INI(close);
    HOOK_FUNC_INI(fcntl);
    HOOK_FUNC_INI(ioctl);
    HOOK_FUNC_INI(getsockopt);
    HOOK_FUNC_INI(setsockopt);
}

#undef HOOK_FUNC_INIT

static uint64_t s_connect_timeout = -1;
struct _HookIniter
{
    _HookIniter()
    {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();

        g_tcp_connect_timeout->addListener([](const int& oldValue, const int& newValue)
            {
                LOG_INFO(g_logger) << "tcp connect timeout changed from " << oldValue
                    << " to " << newValue;
                
                s_connect_timeout = newValue;
            }
        );
    }
};

static _HookIniter s_hook_initer;

struct timer_info 
{
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hookFunc, uint32_t event, int timeout_so, Args&&... args)
{
    // not hook enabled, call original func
    if(!Hook::IsHookEnabled())
        return fun(fd, std::forward<Args>(args)...);
    
    FDItem::ptr fdi = FDMgr::GetInstance()->get(fd);
    if(!fdi)
        return fun(fd, std::forward<Args>(args)...);

    // fd closed, return
    if(fdi->isClosed())
    {
        errno = EBADF;
        return -1;
    }

    // fd not cloes, but user set it non block, return directly
    if(!fdi->isClosed() || fdi->getUserNonblock())
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    // get io set timeout
    uint64_t fdTimeout = fdi->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

    ssize_t ret = 0;
    while(ret >= 0)
    {
        ret = fun(fd, std::forward<Args>(args)...);
        if(ret >= 0) return ret;    // success, return
        if(ret < 0 && errno == EINTR)   continue;   // errno == interrupted, recall directly
        if(ret < 0 && errno != EINTR && errno != EAGAIN) 
        {
            LOG_ERROR(g_logger) << "io error, errno = " << errno << ", msg = " << strerror(errno);
            return ret;
        }

        // errno == EAGAIN, readd event to epoll instance
        auto ioManager = EpollScheduler::GetThis();
        TimerFunc::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if(fdTimeout != (uint64_t)-1)
        {
            // user set fd timeout, add conditionTimer to remove event from epoll
            timer = ioManager->addConditionTimer(fdTimeout, [winfo, fd, ioManager, event]
            {
                auto t = winfo.lock();
                if(!t || t->cancelled)  return;

                t->cancelled = ETIMEDOUT;
                // TODO:
                // ioManager->triggerAndCancelEvent(fd, (EpollScheduler::EventType)event);
            }, winfo);
        }

        //TODO:
        Coroutine::Yield2Hold();
        if(timer) timer->cancel();
        if(tinfo->cancelled)
        {
            // fd event has been timedout
            errno = tinfo->cancelled;
            return -1;
        }
    }

    return ret;
}

extern "C"
{
unsigned int sleep(unsigned int seconds)
{
    if(!Hook::IsHookEnabled())  return sleep_f(seconds);

    Coroutine::ptr coroutine = Coroutine::GetThreadCurrCoroutine();
    EpollScheduler::ptr ioManager = EpollScheduler::GetThis();
    ioManager->addTimer(seconds * 1000, 
        std::bind((void(Scheduler::*)(Coroutine::ptr, int thread))&EpollScheduler::schedule, ioManager, coroutine, -1));
    
    Coroutine::Yield2Hold();
    return 0;
}

int nanosleep(const struct timespec* req, struct timespec* rem)
{
    if(!Hook::IsHookEnabled())  return nanosleep_f(req, rem);

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
    Coroutine::ptr coroutine = Coroutine::GetThreadCurrCoroutine();
    EpollScheduler::ptr ioManager = EpollScheduler::GetThis();
    ioManager->addTimer(timeout_ms, std::bind((void(Scheduler::*)(Coroutine::ptr, int thread))&EpollScheduler::schedule, ioManager, coroutine, -1));
    Coroutine::Yield2Hold();
}

int socket(int domain, int type, int protocol)
{
    if(!Hook::IsHookEnabled())  return socket_f(domain, type, protocol);

    int fd = socket_f(domain, type, protocol);
    if(fd == -1)    return fd;

    FDMgr::GetInstance()->get(fd, true);
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms)
{
    if(!Hook::IsHookEnabled())  return connect_f(fd, addr, addrlen);
    
    FDItem::ptr fdi = FDMgr::GetInstance()->get(fd);
    if(!fdi || fdi->isClosed())
    {
        errno = EBADF;
        return -1;
    }

    if(!fdi->isSocket())
        return connect_f(fd, addr, addrlen);
    
    if(fdi->getUserNonblock())
        return connect_f(fd, addr, addrlen);

    int ret = connect_f(fd, addr, addrlen);
    if(ret == 0 || (ret < 0 && errno != EINPROGRESS))
        return ret;
    
    EpollScheduler::ptr ioManager = EpollScheduler::GetThis();
    TimerFunc::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1)
    {
        timer = ioManager->addConditionTimer(
            timeout_ms, [winfo, fd, ioManager]()
            {
                auto t = winfo.lock();
                if(!t || t->cancelled)  return;

                t->cancelled = ETIMEDOUT;
                ioManager->triggerAndCancelEvent(fd, EpollScheduler::EventType::WRITE);
            }, winfo
        );
    }

    ret = ioManager->addEvent(fd, EpollScheduler::WRITE);
    if(ret == 0)
    {
        Coroutine::Yield2Hold();
        if(timer)   timer->cancel();
        if(tinfo->cancelled)   
        {
            errno = tinfo->cancelled;
            return -1;
        }
    }
    else
    {
        if(timer)   timer->cancel();
        LOG_ERROR(g_logger) << "connect addEvent (" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len))
        return -1;
    
    errno = error;
    return !error ? 0 : -1;
    
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return connect_with_timeout(sockfd, addr, addrlen, s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(s, accept_f, "accept", EpollScheduler::EventType::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        FDMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", EpollScheduler::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", EpollScheduler::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", EpollScheduler::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", EpollScheduler::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", EpollScheduler::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", EpollScheduler::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", EpollScheduler::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", EpollScheduler::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", EpollScheduler::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", EpollScheduler::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd)
{
    if(!Hook::IsHookEnabled())  return close_f(fd);
    
    FDItem::ptr fdi = FDMgr::GetInstance()->get(fd);
    if(fdi)
    {
        auto ioManager = EpollScheduler::GetThis();
        if(ioManager)
            ioManager->triggerAndCancelEvent(fd);
        
        FDMgr::GetInstance()->del(fd);
    }

    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                FDItem::ptr ctx = FDMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                FDItem::ptr ctx = FDMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int fd, unsigned long int request, ...)
{
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request)
    {
        bool user_nonblock = !!*(int*)arg;
        FDItem::ptr fdi = FDMgr::GetInstance()->get(fd);
        if(!fdi || fdi->isClosed() || !fdi->isSocket())
        {
            return ioctl_f(fd, request, arg);
        }
        fdi->setUserNonblock(user_nonblock);
    }

    return ioctl_f(fd, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!Hook::IsHookEnabled())  return setsockopt_f(sockfd, level, optname, optval, optlen);

    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            FDItem::ptr ctx = FDMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

};
#endif