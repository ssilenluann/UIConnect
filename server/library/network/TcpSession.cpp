#ifndef NETWORK_TCPSESSION_CPP
#define NETWORK_TCPSESSION_CPP

#include "TcpSession.h"
#include "./event/EventLoop.h"
#include "../log/Logger.h"
#include "../reactor/Processor.h"
#include <iostream>

static Logger::ptr g_logger = LOG_NAME("system");

TcpSession::TcpSession(unsigned long sessionId, std::shared_ptr<TcpConnection> connection, std::shared_ptr<EventLoop>& loop)
: m_sessionId(sessionId), m_loop(loop), 
    m_activeTime(std::chrono::high_resolution_clock::now())
{
    m_connection = connection;
    m_msgSize = 0;
}

TcpSession::~TcpSession()
{
    // removeConnection(m_connection->fd());
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

    // lifeControl();
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

bool TcpSession::removeConnection(SOCKET socket)
{
    m_connection->onClose();
    if(!m_connection->isClosed())   return false;
    
    m_connection.reset();

    if(m_closeCallback) m_closeCallback(m_sessionId);

    return true;
}

// make sure the connection is deconstruction in controll thread
bool TcpSession::removeConnectionInLoop(SOCKET socket)
{
    if(m_loop.expired())
    {
        LOG_INFO(g_logger) << "session is still alive but event loop got killed";
        return false;
    }

    m_loop.lock()->addTask(std::bind(&TcpSession::removeConnection, this, socket));
}
void TcpSession::lifeControl()
{
    // timeout and rate control
    m_loop.lock()->addTimer(SESSION_TIMEOUT / 2,
        [&]()
        {
            int duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_activeTime).count();
            if(duration > SESSION_TIMEOUT || m_msgSize > SESSION_RATE)
            {
                LOG_INFO(g_logger) << "timeout or send too fast";
                m_loop.lock()->removeSession(m_sessionId);
                // removeConnection(m_connection->fd());
            }

            m_activeTime = std::chrono::high_resolution_clock::now();
            m_msgSize = 0;
        }, false
    );
}
#endif
