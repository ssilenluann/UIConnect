#ifndef NETWORK_EPOLL_EPOLL_CPP
#define NETWORK_EPOLL_EPOLL_CPP

#include "Epoll.h"

Epoll::Epoll(): m_epfd(INVALID_SOCKET), m_events(EPOLL_EVENT_ADD_SIZE)
{
    m_epfd = epoll_create(1);
    if(m_epfd < 0)
        m_epfd = INVALID_SOCKET;

}

Epoll::~Epoll()
{

}

int Epoll::addIn(SOCKET fd)
{
    return ctrl(fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLPRI);
}

int Epoll::modIn(SOCKET fd)
{
    return ctrl(fd, EPOLL_CTL_MOD, EPOLLIN | EPOLLPRI);
}

int Epoll::del(SOCKET fd)
{
    return ctrl(fd, EPOLL_CTL_DEL, 0);
}

int Epoll::addOut(SOCKET fd)
{
    return ctrl(fd, EPOLL_CTL_ADD, EPOLLOUT);
}

int Epoll::modOut(SOCKET fd)
{
    return ctrl(fd, EPOLL_CTL_MOD, EPOLLOUT);
}

int Epoll::addInOut(SOCKET fd)
{
    return ctrl(fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLPRI);
}

int Epoll::modInOut(SOCKET fd)
{
    return ctrl(fd, EPOLL_CTL_MOD, EPOLLIN | EPOLLOUT | EPOLLPRI);
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
        std::shared_ptr<TcpChannel>* channel = static_cast<std::shared_ptr<TcpChannel>*>(m_events[i].data.ptr);
        if(m_channels.find((*channel)->fd()) == m_channels.end() || 
        m_channels[(*channel)->fd()].get() != channel->get())
        {
            //TODO: LOG
            continue;
        }

        activeChannels.emplace_back(*channel);
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
    event.data.ptr = &pChannel;
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
    
    return ctrl(m_channels[fd], op, eventType) >= 0;
}

bool Epoll::addChannel(std::shared_ptr<TcpChannel>& channel)
{
    return updateChannel(EPOLL_CTL_ADD, channel);
}

bool Epoll::delChannel(std::shared_ptr<TcpChannel>& channel)
{
    return updateChannel(EPOLL_CTL_DEL, channel);
}

bool Epoll::modChannel(std::shared_ptr<TcpChannel>& channel)
{
    return updateChannel(EPOLL_CTL_MOD, channel);
}

// @param: action: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
bool Epoll::updateChannel(int action, std::shared_ptr<TcpChannel>& channel)
{

    if(m_channels.find(channel->fd()) != m_channels.end())
    {
        //TODO: LOG
        return false;
    }

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