#ifndef NETWORK_TCPCONNECTION_CPP
#define NETWORK_TCPCONNECTION_CPP

#include "TcpConnection.h"
#include "../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

TcpConnection::TcpConnection(SOCKET fd, std::shared_ptr<EventLoop> loop)
: m_state(ConnState::Disconnected), m_readBuffer(new Buffer()), 
m_writeBuffer(new Buffer()), m_loop(loop), m_socket(new TcpSocket(fd)), 
m_channel(new TcpChannel(loop, m_socket->fd())){}

TcpConnection::~TcpConnection()
{
    onClose();
}

// not put it in construction function, because of "this"
bool TcpConnection::init()
{
    m_channel->setReadCallback(std::bind(&TcpConnection::onRead, this));
    m_channel->setWriteCallback(std::bind(&TcpConnection::onWrite, this));
    m_channel->setErrorCallback(std::bind(&TcpConnection::onError, this));
    m_channel->setCloseCallback(std::bind(&TcpConnection::onClose, this));
	
	bool retp =  m_channel->addTargetEvent(TcpChannel::readEvent);
    if(retp)
        m_state = ConnState::Connected;
    return retp;
}

void TcpConnection::onRead()
{
    if(m_state != ConnState::Connected) return;

    int size = 0;
    bool retp = m_socket->recv(m_readBuffer, size);
    
    // close
    if(retp == false && size == 0)
    {
        onClose();
        return ;
    }

    // error
    if(retp == false && size == -1)
    {
        // TODO: LOG
        onError();
        return;
    }

    // try to fetch all buffer message, split message into packets, process each message
    Packet packet;

    // get first message
    m_readBuffer->getPack(packet);
    while(packet.isValid())
    {
        // deal with that message
        if(m_readCallback)
            m_readCallback(packet);

        // get next message
        packet.clear();
        m_readBuffer->getPack(packet);
    }
    
}

void TcpConnection::onWrite()
{
    if(m_state != ConnState::Connected) return;

    int size = 0;
    bool retp = false;

    retp = m_socket->send(m_writeBuffer, size);

    // other side closed
    if(retp == false || size == 0)
    {
        LOG_FMT_INFO(g_logger, "other side closed, socket fd = %d, errno = %d", m_socket->fd(), errno);
        onClose();
        return;
    }

    // socket error
    if(retp == false && size < 0)
    {
        LOG_FMT_INFO(g_logger, "socket error, socket fd = %d, errno = %d", m_socket->fd(), errno);
        onError();
        return;
    }

    if(retp == true && size == 0)
    {
        // EINTR OR EWOULDBLOCK
        return;
    }

    // epoll event is in LT mode, if write buffer is empty, remove event
    if(m_writeBuffer->getUnreadSize() == 0)
        m_channel->disableWriting();

    if(m_writeCallback)
        m_writeCallback(size); 
    
}

// just add wirte event to epoll, when event deteached, write data to socket
bool TcpConnection::send(Packet& pack)
{
    m_writeBuffer->setMsg(pack);
    return m_channel->enableWriting();
}

void TcpConnection::onError()
{
    LOG_FMT_INFO(g_logger, "connection error and will be closed, socket fd = %d, errno = %d", m_socket->fd(), errno);
    onClose();
    if(m_errorCallback)
        m_errorCallback(m_socket->fd());
}

void TcpConnection::onClose()
{
    if(m_state == ConnState::Disconnected)
        return;

    m_state = ConnState::Disconnecting;
    m_channel->disable();
    m_socket->close();

    m_state = ConnState::Disconnected;
    if(m_closeCallback)
        m_closeCallback(m_socket->fd());
    
}

void TcpConnection::setReadCallback(PROCESS_FUNC func)
{
	m_readCallback = func;	
}

void TcpConnection::setWriteCallback(WRITE_CALLBACK func)
{
	m_writeCallback = func;
}

void TcpConnection::setErrorCallback(EVENT_CALLBACK func)
{
	m_errorCallback = func;
}

void TcpConnection::setCloseCallback(EVENT_CALLBACK func)
{
	m_closeCallback = func;
}

std::shared_ptr<TcpChannel> TcpConnection::getChannel()
{
	return m_channel;
}

TcpConnection::ConnState TcpConnection::status()
{
    return m_state;
}
#endif
