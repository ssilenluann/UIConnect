#ifndef NETWORK_EPOLL_EPOLL_H
#define NETWORK_EPOLL_EPOLL_H
#include <vector>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include "../socket/TcpSocket.h"
#include "../TcpChannel.h"

#define EPOLL_EVENT_ADD_SIZE 1024

class Epoll
{

    typedef std::vector<epoll_event> EVENT_LIST;
    typedef std::map<SOCKET, std::shared_ptr<TcpChannel>> CHANNEL_MAP;
    typedef std::vector<std::shared_ptr<TcpChannel>> CHANNEL_LIST;
public:

    Epoll(SOCKET fd = INVALID_SOCKET);
    ~Epoll();
    Epoll(const Epoll& epoll) = delete;
    Epoll& operator=(const Epoll& epoll) = delete;

    bool poll(CHANNEL_LIST& activeChannels, int timeout = 1000);

    inline bool isValid();

    bool addChannel(SOCKET fd, std::shared_ptr<TcpChannel>& channel);
    bool delChannel(SOCKET fd, std::shared_ptr<TcpChannel>& channel);
    bool modChannel(SOCKET fd, std::shared_ptr<TcpChannel>& channel);
    bool updateChannel(int action, SOCKET fd, std::shared_ptr<TcpChannel>& channel);

private:
    bool ctrl(std::shared_ptr<TcpChannel>& pChannel, int op, int eventType);
    bool ctrl(SOCKET fd, int op, int eventType);
    

private:
    
    SOCKET m_epfd;
    EVENT_LIST m_events;
    CHANNEL_MAP m_channels;

};
#endif
