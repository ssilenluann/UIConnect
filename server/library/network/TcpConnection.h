#ifndef NETWORK_TCPCONNECTION_H
#define NETWORK_TCPCONNECTION_H

#include <memory>
#include "./socket/Buffer.h"
#include "./socket/Packet.h"
#include "./socket/TcpSocket.h"
#include "EpollChannel.h"
#include "./epoll/EpollScheduler.h"

class TcpConnection
{
    typedef std::shared_ptr<TcpConnection> ptr;
    typedef std::function<void(int socket)> EVENT_CALLBACK;
    typedef std::function<void(int)> WRITE_CALLBACK;
    typedef std::function<void(Packet&)> PROCESS_FUNC;

public:
    TcpConnection(TcpSocket::ptr& sock, std::shared_ptr<EpollScheduler> loop = nullptr);
    ~TcpConnection();

    // TcpConnection(const TcpConnection& connection) = delete;
    TcpConnection& operator=(const TcpConnection& rhs) = delete;
    
    bool init();
    inline int fd() { return m_socket->getSocket();}

    void onRead();
    void onWrite();
    void onError();
    void onClose();

    bool send(Packet& pack);

    void setReadCallback(PROCESS_FUNC func) { m_readCallback = func;}
	void setWriteCallback(WRITE_CALLBACK func) { m_writeCallback = func;}
	void setErrorCallback(EVENT_CALLBACK func) { m_errorCallback = func;}
	void setCloseCallback(EVENT_CALLBACK func) { m_closeCallback = func;}
    enum ConnState {Disconnected = 0, Connecting, Connected, Disconnecting};

	std::shared_ptr<EpollChannel> getChannel() { return m_channel;}
    ConnState status() { return m_state;}
    inline bool isClosed() { return m_state == Disconnected;}
    
private:
    ConnState m_state;

    std::shared_ptr<Buffer> m_readBuffer;
    std::shared_ptr<Buffer> m_writeBuffer;

    std::weak_ptr<EpollScheduler> m_loop;
    std::shared_ptr<TcpSocket> m_socket;
    std::shared_ptr<EpollChannel> m_channel;

    PROCESS_FUNC m_readCallback;
    WRITE_CALLBACK m_writeCallback;
    EVENT_CALLBACK m_errorCallback;
    EVENT_CALLBACK m_closeCallback;

};
#endif
