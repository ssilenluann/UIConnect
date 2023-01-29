#ifndef NETWORK_TCPCHANNEL_H
#define NETWORK_TCPCHANNEL_H
#include <functional>
#include <memory>
#include "./socket/TcpSocket.h"

class EventLoop;
class TcpChannel: public std::enable_shared_from_this<TcpChannel>
{
    typedef std::function<void()> EventCallback;

public:
    TcpChannel(std::weak_ptr<EventLoop> loop, SOCKET fd);

    void setReadCallback(EventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setCloseCallback(EventCallback cb);
    void setErrorCallback(EventCallback cb);

    bool enableReading();
    bool enableWriting();
    bool disableReading();
    bool disableWriting();
    bool disable();

    bool isNoneEvent();
    int targetEvent();

    std::weak_ptr<EventLoop> eventLoop();

    SOCKET fd();

    bool handleEvent();

    bool setTargetEvent(int targetEvent);
    bool addTargetEvent(int targetEvent);

public:
    static const int noneEvent;
    static const int readEvent;
    static const int writeEvent;

private:
    SOCKET m_fd;

    int m_targetEvent;
    int m_activeEvent;

    EventCallback m_onRead;
    EventCallback m_onWrite;
    EventCallback m_onClose;
    EventCallback m_onError;

    std::weak_ptr<EventLoop> m_ownerLoop;
    
};
#endif
