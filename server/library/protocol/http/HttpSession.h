#ifndef PROTOCOL_HTTP_HTTPSESSION_H
#define PROTOCOL_HTTP_HTTPSESSION_H

#include <memory>
#include "../../network/TcpSession.h"
#include "HttpConnection.h"
#include "HttpRequest.h"

class HttpSession: public TcpSession
{
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(unsigned long sessionId, std::shared_ptr<HttpConnection> connection, std::shared_ptr<EventLoop>& loop);
    virtual bool init() override;

    void handleRequest(std::shared_ptr<HttpRequest> req);
protected:
    std::shared_ptr<HttpConnection> m_connection;

};
#endif