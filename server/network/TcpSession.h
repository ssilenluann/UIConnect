#ifndef NETWORK_TCPSESSION_H
#define NETWORK_TCPSESSION_H

#include "TcpConnection.h"
#include <thread>
class TcpSession
{
public:
    TcpSession(unsigned long sessionId, std::unique_ptr<TcpConnection> connection, std::shared_ptr<EventLoop>& loop);
    virtual ~TcpSession();

    TcpSession(const TcpSession& session) = delete;
    TcpSession& operator=(const TcpSession& rhs) = delete;

    bool init();
    void send(Packet& pack);
    void handleMessage(Packet& pack);
    bool removeConnection(SOCKET socket);
    bool removeConnectionInLoop(SOCKET socket);
    inline unsigned long id() {return m_sessionId;}

    void setCloseCallback(std::function<void(SOCKET)> func)
    {
        m_closeCallback = func;
    }


protected:
    unsigned long m_sessionId;
    std::weak_ptr<EventLoop> m_loop;
    std::unique_ptr<TcpConnection> m_connection;
    std::function<void(SOCKET)> m_closeCallback;
    
};
#endif