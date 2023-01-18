#ifndef NETWORK_TCPCONNECTION_H
#define NETWORK_TCPCONNECTION_H

#include <memory>
#include "./socket/Buffer.h"
#include "./socket/TcpSocket.h"
#include "TcpChannel.h"
class TcpConnection
{

    typedef std::function<void()> EVENT_CALLBACK;
    typedef std::function<void(Packet&)> PROCESS_FUNC;

public:
    TcpConnection();
    ~TcpConnection();

    TcpConnection(const TcpConnection& connection) = delete;
    TcpConnection& operator=(const TcpConnection& rhs) = delete;
    
    void init();
private:
    enum ConnState {Disconnected = 0, Connecting, Connected, Disconnecting};

    void onRead();
    void onWrite();
    void onError();
    void onClose();

private:
    ConnState m_state;

    std::shared_ptr<Buffer> m_readBuffer;
    std::shared_ptr<Buffer> m_writeBuffer;

    std::weak_ptr<EventLoop> m_loop;
    std::shared_ptr<TcpSocket> m_socket;
    std::shared_ptr<TcpChannel> m_channel;

    PROCESS_FUNC m_readCallback;
    EVENT_CALLBACK m_writeCallback;
    EVENT_CALLBACK m_errorCallback;
    EVENT_CALLBACK m_closeCallback;

};
#endif