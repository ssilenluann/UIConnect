#ifndef PROTOCOL_HTTP_HTTPCONNECTION_H
#define PROTOCOL_HTTP_HTTPCONNECTION_H

#include <memory>
#include "../../network/TcpConnection.h"
#include "HttpResponse.h"
#include "HttpRequest.h"

struct HttpResult
{
    typedef std::shared_ptr<HttpResult> ptr;

    enum class Error
    {
        OK = 0,
        INVALID_URL = 1,
        INVALID_HOST = 2,
        CONNECT_FAIL = 3,
        SEND_CLOSE_BY_PEER = 4,
        SEND_SOCKET_ERROR = 5,
        TIMEOUT = 6,
        CREATE_SOCKET_ERROR = 7,
        POOL_GET_CONNECTION = 8,
        POOL_INVALID_CONNECTION = 9,
    };

    HttpResult(int result, HttpResponse::ptr& error, const std::string& info) {}

    int result;
    HttpResponse::ptr response;
    std::string error;
    std::string toString() const;
};

// http client class
class HttpConnection: public TcpConnection
{
public:
    typedef std::function<void(std::shared_ptr<HttpRequest>)> HTTP_READ_CB;

    HttpConnection(SOCKET fd = INVALID_SOCKET, std::shared_ptr<EventLoop> loop = nullptr);
    
    virtual void onRead() override;
    virtual bool init() override;
    void setReadCallback(HTTP_READ_CB func);
    bool send(std::shared_ptr<HttpResponse>& res);

    void onParseRequestError(std::string& msg);
    void onParseResponseError(std::string& msg);

    void resetIncompReqCnt();
    int getIncompReqCnt();
protected:
    HTTP_READ_CB m_readCallback;
    int m_incompleteReqCnt;
};


#endif