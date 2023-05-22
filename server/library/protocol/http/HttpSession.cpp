
#include "HttpSession.h"
#include "../../reactor/HttpDispatcher.h"
#include "../../log/Logger.h"
#include "../config/Config.h"
#include "../../network/event/EventLoop.h"

static Logger::ptr g_logger = LOG_NAME("system");
static ConfigItem<int>::ptr g_session_timeout = Config::SearchOrAdd("tcp.session.timeout", 30000, "session time out");
static ConfigItem<int>::ptr g_session_msg_handle_size = Config::SearchOrAdd("tcp.session.msg.handle_size", 6000, "session msg handle size");
static ConfigItem<int>::ptr g_session_msg_incomplete_cnt = Config::SearchOrAdd("http.session.msg.incomplete_cnt", 20, "session msg incomplete count");

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

    // lifeControl(); // close for ab test
    return true;
}

void HttpSession::handleRequest(std::shared_ptr<HttpRequest> req)
{
    m_activeTime = std::chrono::high_resolution_clock::now();
    m_processedMsgSize += req->getSize();

    std::shared_ptr<HttpResponse> res(new HttpResponse());
    HttpDispatcher::Instance().dispatch(req, res);
    m_connection->send(res);

    if(req->getHeader("Connection", "") == "")
    {
        m_connection->setWriteCallback(
            [&](int size)
            {
                m_loop.lock()->removeSession(m_sessionId);
            }
        );
    }
}

// timeout and rate control
void HttpSession::lifeControl()
{
    int timeout = g_session_timeout->getValue();
    m_loop.lock()->addTimer(timeout, std::bind(&HttpSession::checkValid, this));
}

void HttpSession::checkValid()
{
    // timeout and rate control
    int timeout = g_session_timeout->getValue();
    int handleMsgSize = g_session_msg_handle_size->getValue();

    if(m_connection->getIncompReqCnt() > g_session_msg_incomplete_cnt->getValue())
    {
        LOG_INFO(g_logger) << "too many incomplete message, maybe send too slow";
        m_loop.lock()->removeSession(m_sessionId);
        return;                
    }

    int duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_activeTime).count();
    if(duration > timeout || m_processedMsgSize > handleMsgSize)
    {
        LOG_INFO(g_logger) << "timeout or send too fast";
        m_loop.lock()->removeSession(m_sessionId);
        return;
    }

    m_activeTime = std::chrono::high_resolution_clock::now();
    m_processedMsgSize = 0;
    m_loop.lock()->addTimer(timeout, std::bind(&HttpSession::checkValid, this));
}
