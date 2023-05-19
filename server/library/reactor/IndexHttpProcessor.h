#ifndef REACTOR_INDEXHTTPPROCESSOR_H
#define REACTOR_INDEXHTTPPROCESSOR_H

#include "HttpProcessor.h"
class IndexHttpProcessor: public HttpProcessor
{
public:
    IndexHttpProcessor() = default;
    virtual void handleHttpRequest(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> res);
};

#endif