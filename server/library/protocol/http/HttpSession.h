#ifndef PROTOCOL_HTTP_HTTPSESSION_H
#define PROTOCOL_HTTP_HTTPSESSION_H

#include <memory>

#include "HttpRequest.h"
#include "../stream/SocketStream.h"

class HttpSession: public SocketStream
{
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock);

    /**
     * @brief recv http request
    */
    HttpRequest::ptr recvRequest();

    /**
     * @brief send http response
     * @param[in] rsp Http response
     * @return 
     *      @retval > 0 send success
     *      @retval = 0 connection close
     *      @retval < 0 socket error
    */
    int sendResponse(HttpResponse::ptr rsp);
};

#endif