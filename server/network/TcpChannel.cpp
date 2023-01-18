#ifndef NETWORK_TCPCHANNEL_CPP
#define NETWORK_TCPCHANNEL_CPP

#include <sys/epoll.h>

#include "TcpChannel.h"
#include "./event/EventLoop.h"

const int TcpChannel::noneEvent = 0;
const int TcpChannel::readEvent = EPOLLIN | EPOLLPRI;
const int TcpChannel::writeEvent = EPOLLOUT;

TcpChannel::TcpChannel(std::weak_ptr<EventLoop> loop, SOCKET fd)
: m_fd(fd), m_targetEvent(noneEvent), m_activeEvent(noneEvent), m_ownerLoop(loop)
{
    
}

void TcpChannel::setReadCallback(EventCallback cb)
{
    m_onRead = cb;
}

void TcpChannel::setWriteCallback(EventCallback cb)
{
    m_onWrite = cb;
}

void TcpChannel::setCloseCallback(EventCallback cb)
{
    m_onClose = cb;
}

void TcpChannel::setErrorCallback(EventCallback cb)
{
    m_onError = cb;
}

bool TcpChannel::enableReading()
{
    if((m_targetEvent & readEvent) > 0)
        return true;
    
    return addTargetEvent(readEvent);
}

bool TcpChannel::enableWriting()
{
    if((m_targetEvent & writeEvent) > 0)
        return true;
    
    return addTargetEvent(writeEvent);
}

bool TcpChannel::disableReading()
{
    if((m_targetEvent & readEvent) == 0)
        return true;
    
    if((m_targetEvent & writeEvent) == 0)
        return disable();

    return setTargetEvent(writeEvent);
}

bool TcpChannel::disableWriting()
{
    if((m_targetEvent & writeEvent) == 0)
        return true;
    
    if((m_targetEvent & readEvent) == 0)
        return disable();

    return setTargetEvent(readEvent);
}

bool TcpChannel::disable()
{
    auto loop = m_ownerLoop.lock();
    if(loop == nullptr)
        return false;
    
    return loop->updateChannel(m_fd, this, EPOLL_CTL_DEL, 0);
}

bool TcpChannel::isNoneEvent()
{
    return (m_activeEvent | noneEvent) > 0;
} 

int TcpChannel::targetEvent()
{
    return m_targetEvent;
}

std::weak_ptr<EventLoop> TcpChannel::eventLoop()
{
    return m_ownerLoop;
}

SOCKET TcpChannel::fd()
{
    return m_fd;
}

bool TcpChannel::setTargetEvent(int targetEvent)
{
    auto loop = m_ownerLoop.lock();
    if(loop == nullptr)
        return false;
    
    return loop->updateChannel(m_fd, this, EPOLL_CTL_MOD, targetEvent);
}

bool TcpChannel::addTargetEvent(int targetEvent)
{
    auto loop = m_ownerLoop.lock();
    if(loop == nullptr)
        return false;

    return loop->updateChannel(m_fd, this, EPOLL_CTL_ADD, targetEvent);
}

bool TcpChannel::handleEvent()
{
    //TODO: log
    // EPOLLHUP: read and write for close both disable, maybe socket just created, not connect to other side
    if((m_activeEvent & EPOLLHUP) && !(m_activeEvent & EPOLLIN) && m_onClose)
        m_onClose();
    
    if((m_activeEvent & EPOLLERR) && m_onError)
        m_onError();
    
    // EPOLLRDHUP: other close connection
    // EPOLLPRI: urgent read event
    if((m_activeEvent & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) && m_onRead)
        m_onRead();

    if((m_activeEvent & EPOLLOUT) && m_onWrite)
        m_onWrite();

    return true;
}
#endif
