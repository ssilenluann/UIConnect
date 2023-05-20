
#include "HttpSession.h"
#include "../../reactor/HttpDispatcher.h"
#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

HttpSession::HttpSession(unsigned long sessionId, std::shared_ptr<HttpConnection> connection, std::shared_ptr<EventLoop> &loop)
: m_connection(connection), TcpSession(sessionId, connection, loop)
{
}

bool HttpSession::init()
{
    if(!m_connection->init())   
    {
        LOG_FMT_ERROR(g_logger, "tcp session init failed, session id = %d, errno = %d", m_sessionId, errno);
        m_connection.reset();
        return false;
    }

    m_connection->setReadCallback(
        std::bind(&HttpSession::handleRequest, this, std::placeholders::_1)
    );
    m_connection->setCloseCallback(std::bind(&TcpSession::removeConnectionInLoop, this, std::placeholders::_1));

    // lifeControl();
    return true;
}

void HttpSession::handleRequest(std::shared_ptr<HttpRequest> req)
{
    std::shared_ptr<HttpResponse> res(new HttpResponse());
    HttpDispatcher::Instance().Dispatch(req, res);
    m_connection->send(res);
}

