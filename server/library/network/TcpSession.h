#ifndef NETWORK_TCPSESSION_H
#define NETWORK_TCPSESSION_H

#include "TcpConnection.h"
#include "Callback.h"
#include <atomic>
#include <chrono>

#define SESSION_TIMEOUT 3000
#define SESSION_RATE 3000

class EventLoop;
class TcpSession: public std::enable_shared_from_this<TcpSession>
{
public:
    TcpSession(unsigned long sessionId, std::shared_ptr<TcpConnection> connection, std::shared_ptr<EventLoop>& loop);
    virtual ~TcpSession();

    TcpSession(const TcpSession& session) = delete;
    TcpSession& operator=(const TcpSession& rhs) = delete;
    
    virtual bool init();
    void send(Packet& pack);
    void handleMessage(Packet& pack);
    bool removeConnection(SOCKET socket);
    bool removeConnectionInLoop(SOCKET socket);
    inline unsigned long id() {return m_sessionId;}

    virtual void lifeControl();
    virtual void checkValid();
    
protected:
    
    unsigned long m_sessionId;
    std::weak_ptr<EventLoop> m_loop;
    std::shared_ptr<TcpConnection> m_connection;
    EVENT_CALLBACK m_closeCallback;

    std::chrono::time_point<std::chrono::high_resolution_clock>  m_activeTime;
    std::atomic<unsigned long long> m_processedMsgSize;
    
};
#endif