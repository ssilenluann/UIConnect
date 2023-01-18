#ifndef NETWORK_EPOLL_EPOLL_H
#define NETWORK_EPOLL_EPOLL_H
#include <vector>
#include <map>
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

    Epoll();
    ~Epoll();
    Epoll(const Epoll& epoll) = delete;
    Epoll& operator=(const Epoll& epoll) = delete;

    int addIn(SOCKET fd);
    int modIn(SOCKET fd);
    int del(SOCKET fd);
    
    int addOut(SOCKET fd);
    int modOut(SOCKET fd);
    
    int addInOut(SOCKET fd);
    int modInOut(SOCKET fd);

    bool poll(CHANNEL_LIST& activeChannels, int timeout = 1000);

    inline bool isValid();

    bool addChannel(std::shared_ptr<TcpChannel>& channel);
    bool delChannel(std::shared_ptr<TcpChannel>& channel);
    bool modChannel(std::shared_ptr<TcpChannel>& channel);

    bool ctrl(std::shared_ptr<TcpChannel>& pChannel, int op, int eventType);
    bool ctrl(SOCKET fd, int op, int eventType);

private:
    bool updateChannel(int action, std::shared_ptr<TcpChannel>& channel);

private:
    
    SOCKET m_epfd;
    EVENT_LIST m_events;
    CHANNEL_MAP m_channels;

};
#endif