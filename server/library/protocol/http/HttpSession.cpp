
#include "HttpSession.h"
#include "../../reactor/HttpDispatcher.h"
HttpSession::HttpSession(unsigned long sessionId, std::shared_ptr<HttpConnection> connection, std::shared_ptr<EventLoop> &loop)
: TcpSession(sessionId, connection, loop)
{
}

bool HttpSession::init()
{
    TcpSession::init();
    m_connection->setReadCallback(
        std::bind(&HttpSession::handleRequest, this, std::placeholders::_1)
    );

}

void HttpSession::handleRequest(std::shared_ptr<HttpRequest> req)
{
    std::shared_ptr<HttpResponse> res(new HttpResponse());
    HttpDispatcher::Instance().Dispatch(req, res);
    m_connection->send(res);
}

