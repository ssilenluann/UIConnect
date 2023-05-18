#ifndef NETWORK_TCPSESSION_CPP
#define NETWORK_TCPSESSION_CPP

#include "TcpSession.h"
#include "./epoll/EpollWorker.h"
#include "../log/Logger.h"

#include <iostream>

static Logger::ptr g_logger = LOG_NAME("system");

TcpSession::TcpSession(unsigned long sessionId, std::unique_ptr<TcpConnection> connection, std::shared_ptr<EpollWorker>& worker)
: m_sessionId(sessionId), m_epollWorker(worker)
{
    m_connection.reset(connection.release());
    m_connection->init();
}

TcpSession::~TcpSession()
{
    // removeConnection();
}

bool TcpSession::init()
{
    if(!m_connection->init())   
    {
        LOG_FMT_ERROR(g_logger, "tcp session init failed, session id = %d, errno = %d", m_sessionId, errno);
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
    ProcessorProxy::Instance().handleMsg(pack, shared_from_this());
}

bool TcpSession::removeConnection(int socket)
{
    m_connection->onClose();
    if(!m_connection->isClosed())   return false;
    
    m_connection.release();

    if(m_closeCallback) m_closeCallback(m_sessionId);

    return true;
}

// make sure the connection is deconstruction in controll thread
bool TcpSession::removeConnectionInLoop(int socket)
{
    if(m_epollWorker.expired())
    {
        LOG_INFO(g_logger) << "session is still alive but event worker got killed";
        return false;
    }
    //TODO:
    // m_loop.lock()->addTask(std::bind(&TcpSession::removeConnection, this, socket));
}
#endif
