#ifndef NETWORK_EPOLL_EPOLLCHANNEL_CPP
#define NETWORK_EPOLL_EPOLLCHANNEL_CPP

#include <sys/epoll.h>

#include "EpollChannel.h"
#include "./EpollScheduler.h"
#include "../../log/Logger.h"

static Logger::ptr g_logger = LOG_NAME("system");

const int EpollChannel::noneEvent = 0;
const int EpollChannel::readEvent = EPOLLIN | EPOLLHUP | EPOLLET;
const int EpollChannel::writeEvent = EPOLLOUT | EPOLLET;

EpollChannel::EpollChannel(std::weak_ptr<EpollScheduler> scheduler, int fd)
    : m_fd(fd), m_targetEvent(noneEvent), m_activeEvent(noneEvent), m_ownerScheduler(scheduler)
{
}

bool EpollChannel::enableReading()
{
    if((m_targetEvent & readEvent) > 0)
        return true;
    
    return m_targetEvent > 0 ? setTargetEvent(m_targetEvent | readEvent): addTargetEvent(readEvent);
}

bool EpollChannel::enableWriting()
{
    if((m_targetEvent & writeEvent) > 0)
        return true;
    
    return m_targetEvent > 0 ? setTargetEvent(m_targetEvent | writeEvent): addTargetEvent(writeEvent);
}

bool EpollChannel::disableReading()
{
    if((m_targetEvent & readEvent) == 0)
        return true;
    
    if((m_targetEvent & writeEvent) == 0)
        return disable();

    return setTargetEvent(writeEvent);
}

bool EpollChannel::disableWriting()
{
    if((m_targetEvent & writeEvent) == 0)
        return true;
    
    if((m_targetEvent & readEvent) == 0)
        return disable();

    return setTargetEvent(readEvent);
}

bool EpollChannel::disable()
{
    auto loop = m_ownerScheduler.lock();
    if(loop == nullptr)
        return false;
    
    return loop->updateChannel(EPOLL_CTL_DEL, m_fd, shared_from_this(), 0);
}

bool EpollChannel::setTargetEvent(int targetEvent)
{
    auto loop = m_ownerScheduler.lock();
    if(loop == nullptr)
        return false;
    
    bool retp =  loop->updateChannel(EPOLL_CTL_MOD, m_fd, shared_from_this(), targetEvent);
	if(!retp)
	{
	    LOG_FMT_ERROR(g_logger, "epoll update channel failed, socket fd = %d, errno = %d", m_fd, errno);
		return false;
	}

	m_targetEvent = targetEvent;
	return true;
}

bool EpollChannel::addTargetEvent(int targetEvent)
{
    auto loop = m_ownerScheduler.lock();
    if(loop == nullptr)
        return false;

    int event = m_targetEvent;
    m_targetEvent |= targetEvent;

    bool retp = loop->updateChannel(EPOLL_CTL_ADD, m_fd, shared_from_this(), targetEvent);
	if(!retp)
	{
        m_targetEvent = event;
		LOG_FMT_ERROR(g_logger, "epoll update channel failed, socket fd = %d, errno = %d", m_fd, errno);

		return false;
	}

	return true;
}

// handle epoll signals
bool EpollChannel::handleEvent()
{
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
