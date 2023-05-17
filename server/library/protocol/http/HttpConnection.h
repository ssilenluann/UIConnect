#ifndef PROTOCOL_HTTP_HTTPCONNECTION_H
#define PROTOCOL_HTTP_HTTPCONNECTION_H

#include <memory>

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

    HttpResult(int result, HttpResponse::ptr& error, const std::string& error) {}

    int result;
    HttpResponse::ptr response;
    std::string error;
    std::string toString() const;
};

// http client class
class HttpConnection
{
public:
    
};


#endif