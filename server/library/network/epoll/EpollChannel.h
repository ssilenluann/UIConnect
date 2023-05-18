#ifndef NETWORK_EPOLL_EPOLLCHANNEL_H
#define NETWORK_EPOLL_EPOLLCHANNEL_H

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "../socket/Packet.h"

class EpollWorker;

class EpollChannel: public std::enable_shared_from_this<EpollChannel>
{
public:
    typedef std::shared_ptr<EpollChannel> ptr;
    typedef std::function<void()> EventCallback;

    EpollChannel(std::weak_ptr<EpollWorker> worker, int fd);
    virtual ~EpollChannel(){}

    void setReadCallback(EventCallback cb) { m_onRead = cb;}
    void setWriteCallback(EventCallback cb) { m_onWrite = cb;}
    void setCloseCallback(EventCallback cb) { m_onClose = cb;}
    void setErrorCallback(EventCallback cb) { m_onError = cb;}

    inline int fd() { return m_fd;}
    inline bool isNoneEvent() { return (m_activeEvent | noneEvent) > 0;}
    inline int targetEvent() { return m_targetEvent;}
    inline void setActiveEvent(int activeEvent) { m_activeEvent = activeEvent;}
    inline std::weak_ptr<EpollWorker> scheduler() { return m_worker;}

    virtual bool handleEvent();

    bool enableReading();
    bool enableWriting();
    bool disableReading();
    bool disableWriting();
    bool disable();

    bool setTargetEvent(uint32_t targetEvent);
    bool addTargetEvent(uint32_t targetEvent);

    static const uint32_t noneEvent;
    static const uint32_t readEvent;
    static const uint32_t writeEvent;

protected:
    int m_fd;
    uint32_t m_targetEvent;      // listening events in epoll instance
    uint32_t m_activeEvent;      // events detacted by epoll instance
    EventCallback m_onRead;
    EventCallback m_onWrite;
    EventCallback m_onClose;
    EventCallback m_onError;
    std::weak_ptr<EpollWorker> m_worker;
};
#endif