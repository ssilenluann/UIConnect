#ifndef STREAM_SOCKETSTREAM_CPP
#define STREAM_SOCKETSTREAM_CPP

#include "SocketStream.h"

SocketStream::SocketStream(Socket::ptr sock) : m_socket(sock)
{
}

SocketStream::~SocketStream()
{
    if(m_socket)    m_socket->close();
}

int SocketStream::read(void *buffer, size_t length)
{
    if(!isConnected())  return -1;

    return m_socket->recv(buffer, length);
}

int SocketStream::read(ByteArray::ptr ba, size_t length)
{
    if(!isConnected())  return -1;

    std::vector<iovec> iovs;
    ba->getWriteBuffers(iovs, length);
    int ret = m_socket->recv(&iovs[0], iovs.size());
    if(ret > 0)
    {
        ba->setPosition(ba->getPosition() + ret);
    }

    return ret;
}

int SocketStream::write(void *buffer, size_t length)
{
    if(!isConnected())  return -1;

    return m_socket->send(buffer, length);
}

int SocketStream::write(ByteArray::ptr ba, size_t length)
{
    if(!isConnected())  return -1;

    std::vector<iovec> iovs;
    ba->getReadBuffers(iovs, length);
    int ret = m_socket->send(&iovs[0], iovs.size());
    if(ret > 0)
    {
        ba->setPosition(ret + ba->getPosition());
    }

    return ret;
}

bool SocketStream::isConnected() const
{
    return m_socket && m_socket->isConnected();
}

Address::ptr SocketStream::getRemoteAddress()
{
    return m_socket ? m_socket->getRemoteAddress() : nullptr;
}

Address::ptr SocketStream::getLocalAddress()
{
    return m_socket ? m_socket->getLocalAddress() : nullptr;
}

std::string SocketStream::getRemoteAddressString()
{
    return m_socket ? m_socket->getRemoteAddress()->toString() : "";
}

std::string SocketStream::getLocalAddressString()
{
    return m_socket ? m_socket->getLocalAddress()->toString() : "";
}

#endif

