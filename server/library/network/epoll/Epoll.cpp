#ifndef NETWORK_EPOLL_EPOLL_CPP
#define NETWORK_EPOLL_EPOLL_CPP

#include "Epoll.h"
#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

Epoll::Epoll(): m_epfd(-1)
{
	m_epfd = epoll_create(1);
    if(m_epfd < 0)
    {
        m_epfd = -1;
        LOG_FATAL(g_logger) << "create epoll fd error\r\n";
    }

    m_events.reserve(EPOLL_EVENT_ADD_SIZE);
}

Epoll::~Epoll()
{
    close();
}

bool Epoll::updateChannel(int action, int fd, std::shared_ptr<EpollChannel>& channel, uint32_t event)
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

int Epoll::poll(CHANNEL_VEC& activeChannels, int timeout)
{
    m_events.clear();
    int size = m_events.capacity();
    int eventNum = 0;

    for(;;)
    {
        eventNum = epoll_wait(m_epfd, &m_events[0], size, timeout);
        if(eventNum == -1 && errno != EINTR && errno != EAGAIN)
        {
            LOG_FMT_ERROR(g_logger, "epoll wait error, fd = %d, errno = %d, info = %s", m_epfd, errno, strerror(errno));
            return eventNum;
        }
        if(eventNum == -1 && (errno == EINTR || errno == EAGAIN))
        {
            LOG_FMT_ERROR(g_logger, "epoll wait error, fd = %d, errno = %d, info = %s", m_epfd, errno, strerror(errno));
            continue;
        }
        break;
    }
   
    // if event list fulled, enlarge it
    if(eventNum == m_events.capacity())
        m_events.resize(eventNum*2);
    
    for(int i = 0; i < eventNum; i++)
    {
        if(m_channels.find(m_events[i].data.fd) == m_channels.end())
        {
        
            LOG_FMT_INFO(g_logger, "unknown active channels, epoll fd = %d, event fd = %d, info = %s", strerror(errno), 
            m_epfd, m_events[i].data.fd);
            continue;
        }

        m_channels[m_events[i].data.fd]->setActiveEvent(m_events[i].events);
        activeChannels.emplace_back(m_channels[m_events[i].data.fd]);
    }

    return eventNum;
}

void Epoll::close()
{
    if(m_epfd == -1)    return;

    LOG_INFO(g_logger) << "epoll exit";
    ::close(m_epfd);
    m_epfd = -1;
    m_events.clear();
    m_channels.clear();
}

bool Epoll::ctrl(EpollChannel::ptr& pChannel, int op, uint32_t eventType)
{
    if(!isValid())  return false;
    
    epoll_event event;
    event.data.fd = pChannel->fd();
    event.events = eventType;
    int retp = epoll_ctl(m_epfd, op, pChannel->fd(), &event);
    if(retp < 0)
    {
        LOG_FMT_ERROR(g_logger, "epoll ctrl error, epoll fd = %d, event fd = %d, errno = %d, info = %s", 
        m_epfd, event.data.fd, errno, strerror(errno));
    }

    return retp >= 0;
}

bool Epoll::ctrl(int fd, int op, uint32_t eventType)
{
    if(m_channels.find(fd) == m_channels.end())
    {
        LOG_INFO(g_logger) << "unknown sock fd";
        return false;
    }
    
    return ctrl(m_channels[fd], op, eventType);
}

#endif
