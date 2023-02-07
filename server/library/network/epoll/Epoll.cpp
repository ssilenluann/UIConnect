#ifndef NETWORK_EPOLL_EPOLL_CPP
#define NETWORK_EPOLL_EPOLL_CPP

#include "Epoll.h"
#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

Epoll::Epoll(): m_epfd(INVALID_SOCKET), m_events(EPOLL_EVENT_ADD_SIZE)
{
	m_epfd = epoll_create(1);
    if(m_epfd < 0)
    {
        m_epfd = INVALID_SOCKET;
        LOG_FATAL(g_logger) << "create epoll fd error\r\n";
    }

}

Epoll::~Epoll()
{
    close();
}

bool Epoll::poll(CHANNEL_LIST& activeChannels, int timeout)
{
    int eventNum = epoll_wait(m_epfd, &*m_events.begin(), static_cast<int>(m_events.size()), timeout);
    if(eventNum == -1 && errno != EINTR)
    {
        LOG_FMT_ERROR(g_logger, "epoll wait error, fd = %d, errno = %d", m_epfd, errno);
        return false;
    }
        
    // if event list fulled, enlarge it
    if(eventNum == m_events.size())
        m_events.resize(eventNum*2);
    
    for(int i = 0; i < eventNum; i++)
    {
        if(m_channels.find(m_events[i].data.fd) == m_channels.end())
        {
        
            LOG_FMT_INFO(g_logger, "unknown active channels, epoll fd = %d, event fd = %d", 
            m_epfd, m_events[i].data.fd);
            continue;
        }

        m_channels[m_events[i].data.fd]->setActiveEvent(m_events[i].events);
        activeChannels.emplace_back(m_channels[m_events[i].data.fd]);
    }

    return true;
}

bool Epoll::isValid()
{
    return m_epfd != INVALID_SOCKET;
}

// LT mode
bool Epoll::ctrl(std::shared_ptr<TcpChannel>& pChannel, int op, int eventType)
{
    if(!isValid())  return false;
    
    epoll_event event;
    event.data.fd = pChannel->fd();
    event.events = eventType;
    int retp = epoll_ctl(m_epfd, op, pChannel->fd(), &event);
    if(retp < 0)
    {
        LOG_FMT_ERROR(g_logger, "epoll ctrl error, epoll fd = %d, event fd = %d, errno = %d", 
        m_epfd, event.data.fd, errno);
    }

    return retp >= 0;
}

bool Epoll::ctrl(SOCKET fd, int op, int eventType)
{
    if(m_channels.find(fd) == m_channels.end())
    {
        LOG_INFO(g_logger) << "unknown sock fd";
        return false;
    }
    
    return ctrl(m_channels[fd], op, eventType);
}

// @param: action: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
bool Epoll::updateChannel(int action, SOCKET fd, std::shared_ptr<TcpChannel>& channel, int event)
{

    // validate
    // NOTICE: maybe it's better to close the existing socket if found
    if(action == EPOLL_CTL_ADD && m_channels.find(fd) != m_channels.end())
    {
        LOG_INFO(g_logger) << "channel is already in epoll's channel list";
        return false;
    }

    // NOTICE: if m_channel[fd] ==  channel, maybe it's better to close both 2 sockets
    if((action == EPOLL_CTL_DEL || action == EPOLL_CTL_MOD) 
        && (m_channels.find(fd) == m_channels.end() || m_channels[fd] != channel))
    {
        LOG_INFO(g_logger) << "unknown socket fd, or channel is wrong";
        return false;
    }

    // epoll controll
    if(!ctrl(channel, action, event))
    {
        return false;
    }

    // add
    if(action == EPOLL_CTL_ADD)
    {
        m_channels[channel->fd()] = channel;
    }

    // del
    if(action == EPOLL_CTL_DEL)
    {
        m_channels.erase(channel->fd());
    }

    return true;
}

void Epoll::close()
{
    if(m_epfd == INVALID_SOCKET)    return;

    LOG_INFO(g_logger) << "epoll exit";
    ::close(m_epfd);
    m_epfd = INVALID_SOCKET;
    m_events.clear();
    m_channels.clear();
}

#endif
