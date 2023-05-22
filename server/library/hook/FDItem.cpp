#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "FDManager.h"

FDItem::FDItem(int fd)
    : m_isInited(false), m_isSocket(false), m_sysNonblock(false), m_userNonblock(false)
    , m_isClosed(false), m_fd(fd), m_recvTimeout(-1), m_sendTimeout(-1)
{
    init();
}

FDItem::~FDItem()
{
}

void FDItem::setTimeout(int type, uint64_t v)
{
    if(type == SO_RCVTIMEO)
        m_recvTimeout = v;
    else
        m_sendTimeout = v;
}

uint64_t FDItem::getTimeout(int type)
{
    return type == SO_RCVTIMEO ? m_recvTimeout : m_sendTimeout;
}

bool FDItem::init()
{
    if(m_isInited)  return true;
    
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    struct stat fd_stat;
    if(fstat(m_fd, &fd_stat) < 0)
    {
        m_isInited = false;
        m_isSocket = false;
    }
    else
    {
        m_isInited = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    if(m_isSocket)
    {
        int flag = fcntl(m_fd, F_GETFL, 0);
        if(!(flag) & O_NONBLOCK)
            fcntl(m_fd, O_NONBLOCK, flag | O_NONBLOCK);
        m_sysNonblock = true;
    }
    else
    {
        m_sysNonblock = false;
    }

    m_userNonblock = false;
    m_isClosed = false;
    return m_isInited;
}
