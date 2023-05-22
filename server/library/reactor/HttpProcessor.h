#ifndef REACTOR_HTTPPROCESSOR_H
#define REACTOR_HTTPPROCESSOR_H
#include "../protocol/http/HttpRequest.h"
#include "../protocol/http/HttpResponse.h"

class HttpProcessor
{
public:
    HttpProcessor() = default;
    virtual void handleHttpRequest(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> res) = 0;
private:

};
#endif