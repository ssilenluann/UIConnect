#ifndef NETWORK_EPOLL_EPOLL_CPP
#define NETWORK_EPOLL_EPOLL_CPP

#include "Epoll.h"

Epoll::Epoll(SOCKET fd): m_epfd(fd), m_events(EPOLL_EVENT_ADD_SIZE)
{
	if(fd != INVALID_SOCKET)
		return;

	m_epfd = epoll_create(1);
    if(m_epfd < 0)
        m_epfd = INVALID_SOCKET;

}

Epoll::~Epoll()
{

}

bool Epoll::poll(CHANNEL_LIST& activeChannels, int timeout)
{
    int eventNum = epoll_wait(m_epfd, &*m_events.begin(), static_cast<int>(m_events.size()), timeout);
    if(eventNum == -1 && errno != EINTR)
        return false;
        
    // if event list fulled, enlarge it
    if(eventNum == m_events.size())
        m_events.resize(eventNum*2);
    
    for(int i = 0; i < eventNum; i++)
    {
        if(m_channels.find(m_events[i].data.fd) == m_channels.end())
        {
            // TODO: here can not find channel by fd, it is nessary to confirm connection is closed
            // TODO: LOG
            continue;
        }

        activeChannels.emplace_back(m_channels[m_events[i].data.fd]);
    }

    return true;
}

bool Epoll::isValid()
{
    return m_epfd != INVALID_SOCKET;
}

bool Epoll::ctrl(std::shared_ptr<TcpChannel>& pChannel, int op, int eventType)
{
    if(!isValid())  return -1;
    
    epoll_event event;
    event.data.fd = pChannel->fd();
    event.events = eventType;
    int retp = epoll_ctl(m_epfd, op, pChannel->fd(), &event);
    if(retp < 0)
    {
        //TODO: LOG

    }

    return retp >= 0;
}

bool Epoll::ctrl(SOCKET fd, int op, int eventType)
{
    if(m_channels.find(fd) == m_channels.end())
    {
        //TODO: LOG
        return false;
    }
    
    return ctrl(m_channels[fd], op, eventType);
}

bool Epoll::addChannel(SOCKET fd, std::shared_ptr<TcpChannel>& channel)
{
    return updateChannel(EPOLL_CTL_ADD, fd, channel);
}

bool Epoll::delChannel(SOCKET fd, std::shared_ptr<TcpChannel>& channel)
{
    return updateChannel(EPOLL_CTL_DEL, fd, channel);
}

bool Epoll::modChannel(SOCKET fd, std::shared_ptr<TcpChannel>& channel)
{
    return updateChannel(EPOLL_CTL_MOD, fd, channel);
}

// @param: action: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
bool Epoll::updateChannel(int action, SOCKET fd, std::shared_ptr<TcpChannel>& channel)
{

    // validate
    // NOTICE: maybe it's better to close the existing socket if found
    if(action == EPOLL_CTL_ADD && m_channels.find(fd) != m_channels.end())
    {
        // TODO: LOG
        return false;
    }

    // NOTICE: if m_channel[fd] ==  channel, maybe it's better to close both 2 sockets
    if((action == EPOLL_CTL_DEL || action == EPOLL_CTL_MOD) 
        && (m_channels.find(fd) == m_channels.end() || m_channels[fd] != channel))
    {
        // TODO: LOG
        return false;
    }

    // epoll controll
    if(!ctrl(channel, action, channel->targetEvent()))
    {
        //TODO: LOG
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

#endif
