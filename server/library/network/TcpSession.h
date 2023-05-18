#ifndef NETWORK_TCPSESSION_H
#define NETWORK_TCPSESSION_H

#include "TcpConnection.h"
#include "Callback.h"
#include "./epoll/EpollWorker.h"
#include "reactor/Processor.h"
#include <memory>
class TcpSession: public std::enable_shared_from_this<TcpSession>
{
public:
    typedef std::shared_ptr<TcpSession> ptr;

    TcpSession(unsigned long sessionId, std::unique_ptr<TcpConnection> connection, std::shared_ptr<EpollWorker>& worker);
    ~TcpSession();

    TcpSession(const TcpSession& session) = delete;
    TcpSession& operator=(const TcpSession& rhs) = delete;
    
    bool init();
    void send(Packet& pack);
    void handleMessage(Packet& pack);
    bool removeConnection(int socket);
    bool removeConnectionInLoop(int socket);
    inline unsigned long id() {return m_sessionId;}
    
private:
    unsigned long m_sessionId;
    std::weak_ptr<EpollWorker> m_epollWorker;
    std::unique_ptr<TcpConnection> m_connection;
    EVENT_CALLBACK m_closeCallback;
};
#endif