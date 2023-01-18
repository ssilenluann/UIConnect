#ifndef NETWORK_TCPCONNECTION_CPP
#define NETWORK_TCPCONNECTION_CPP

#include "TcpConnection.h"

TcpConnection::TcpConnection()
:m_state(ConnState::Disconnected), m_readBuffer(new Buffer()), m_writeBuffer(new Buffer()), m_socket(new TcpSocket()), m_channel(new TcpChannel(m_loop, m_socket->fd())){}

// not put it in construction function, because of "this"
void TcpConnection::init()
{
    m_channel->setReadCallback(std::bind(&TcpConnection::onRead, this));
    m_channel->setWriteCallback(std::bind(&TcpConnection::onWrite, this));
    m_channel->setErrorCallback(std::bind(&TcpConnection::onError, this));
    m_channel->setCloseCallback(std::bind(&TcpConnection::onClose, this));
}

void TcpConnection::onRead()
{
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

    // process message
    Packet packet;
    m_readBuffer->getPack(packet);
    while(packet.isValid())
    {
        if(m_readCallback)
            m_readCallback(packet);
        packet.clear();
        m_readBuffer->getPack(packet);
    }
    
}

void TcpConnection::onWrite()
{
    int size = 0;
    bool retp = m_socket->send(m_writeBuffer, size);

    // other side closed
    if(retp == false || size == 0)
    {
        // TODO: LOG
        onClose();
        return;
    }

    // socket error
    if(retp == false && size < 0)
    {
        //TODO: LOG
        onError();
        return;
    }

    if(retp == true && size > 0)
    {
        // EINTR OR EWOULDBLOCK
        // TODO: LOG
        // TODO: retry
    }

    if(m_writeCallback)
        m_writeCallback();
}

void TcpConnection::onError()
{
    // TODO: LOG
    if(m_errorCallback)
        m_errorCallback();
}

void TcpConnection::onClose()
{
    // TODO: LOG
    if(m_closeCallback)
        m_closeCallback();
}

#endif