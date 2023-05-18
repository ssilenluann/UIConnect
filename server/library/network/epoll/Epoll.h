#ifndef NETWORK_EPOLL_EPOLL_H
#define NETWORK_EPOLL_EPOLL_H

#include <vector>
#include <map>
#include <memory>
#include <sys/epoll.h>

#include "../../Noncopiable.h"
#include "../socket/Socket.h"
#include "EpollChannel.h"

#define EPOLL_EVENT_ADD_SIZE 1024

class Epoll: Noncopyable
{
public:
    typedef std::shared_ptr<Epoll> ptr;
    typedef std::vector<epoll_event> EVENT_VEC;
    typedef std::map<int, EpollChannel::ptr> CHANNEL_MAP;
    typedef std::vector<EpollChannel::ptr> CHANNEL_VEC;

public:

    Epoll();
    ~Epoll();

    /**
     * @brief update listening event of channel in epoll instance
     * @param: action: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
    */
    bool updateChannel(int action, int fd, std::shared_ptr<EpollChannel>& channel, int event);
    
    /**
     * @brief epoll instance start listen ative events
     * @param[out] activeChannels if fd is active, add it to active Channels
     * @param[in] listen timeout
    */
    bool poll(CHANNEL_VEC& activeChannels, int timeout = 1000);

    inline int fd() const { return m_epfd;}
    bool isValid() const { return m_epfd != -1;};
    void close();

private:

    /**
     * @brief update listening event of channel in epoll instance
    */
    bool ctrl(EpollChannel::ptr& pChannel, int op, int eventType);

    /**
     * @brief update listening event of fd
    */
    bool ctrl(int fd, int op, int eventType);
    
private:
    
    int m_epfd;
    EVENT_VEC m_events;
    CHANNEL_MAP m_channels;

};
#endif
