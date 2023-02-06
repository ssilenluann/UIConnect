#ifndef NETWORK_TCPSESSION_CPP
#define NETWORK_TCPSESSION_CPP

#include "TcpSession.h"
#include "./event/EventLoop.h"

#include <iostream>

TcpSession::TcpSession(unsigned long sessionId, std::unique_ptr<TcpConnection> connection, std::shared_ptr<EventLoop>& loop)
: m_sessionId(sessionId), m_loop(loop)
{
    m_connection.reset(connection.release());
}

TcpSession::~TcpSession()
{
    // removeConnection();
}

bool TcpSession::init()
{
    if(!m_connection->init())   
    {
        // TODO: LOG
        m_connection.reset();
        return false;
    }

    m_connection->setReadCallback(std::bind(&TcpSession::handleMessage, this, std::placeholders::_1));
    m_connection->setCloseCallback(std::bind(&TcpSession::removeConnectionInLoop, this, std::placeholders::_1));

    return true;
}

void TcpSession::send(Packet& pack)
{
    m_connection->send(pack);
}

void TcpSession::handleMessage(Packet& pack)
{
    // echo
    std::cout << pack.dataLoad() << std::endl;
    send(pack);
}

bool TcpSession::removeConnection(SOCKET socket)
{
    // TODO: LOG
    m_connection->onClose();
    if(!m_connection->isClosed())   return false;
    
    m_connection.release();

    if(m_closeCallback) m_closeCallback(m_sessionId);

    return true;
}

// make sure the connection is deconstruction in controll thread
bool TcpSession::removeConnectionInLoop(SOCKET socket)
{
    if(m_loop.expired())
    {
        // TODO: LOG, session is still alive but event loop got killed
        return false;
    }

    m_loop.lock()->addTask(std::bind(&TcpSession::removeConnection, this, socket));
}

#endif
