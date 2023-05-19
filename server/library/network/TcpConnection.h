#ifndef NETWORK_TCPCONNECTION_H
#define NETWORK_TCPCONNECTION_H

#include <memory>
#include "./socket/Buffer.h"
#include "./socket/Packet.h"
#include "./socket/TcpSocket.h"
#include "TcpChannel.h"
class TcpConnection
{

    typedef std::function<void(SOCKET socket)> EVENT_CALLBACK;
    typedef std::function<void(int)> WRITE_CALLBACK;
    typedef std::function<void(Packet&)> PROCESS_FUNC;

public:
    TcpConnection(SOCKET fd = INVALID_SOCKET, std::shared_ptr<EventLoop> loop = nullptr);
    ~TcpConnection();

    // TcpConnection(const TcpConnection& connection) = delete;
    TcpConnection& operator=(const TcpConnection& rhs) = delete;
    
    virtual bool init();
    inline SOCKET fd() { return m_socket->fd();}

    virtual void onRead();
    void onWrite();
    void onError();
    void onClose();

    bool send(Packet& pack);

    void setReadCallback(PROCESS_FUNC func);
	void setWriteCallback(WRITE_CALLBACK func);
	void setErrorCallback(EVENT_CALLBACK func);
	void setCloseCallback(EVENT_CALLBACK func);
    enum ConnState {Disconnected = 0, Connecting, Connected, Disconnecting};

	std::shared_ptr<TcpChannel> getChannel();
    ConnState status();

    inline bool isClosed()
    {
        return m_state == Disconnected;
    }
    
protected:
    ConnState m_state;

    std::shared_ptr<Buffer> m_readBuffer;
    std::shared_ptr<Buffer> m_writeBuffer;

    std::weak_ptr<EventLoop> m_loop;
    std::shared_ptr<TcpSocket> m_socket;
    std::shared_ptr<TcpChannel> m_channel;

    PROCESS_FUNC m_readCallback;
    WRITE_CALLBACK m_writeCallback;
    EVENT_CALLBACK m_errorCallback;
    EVENT_CALLBACK m_closeCallback;

};
#endif
