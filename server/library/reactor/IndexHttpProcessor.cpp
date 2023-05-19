#include "IndexHttpProcessor.h"

void IndexHttpProcessor::handleHttpRequest(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> res)
{
    res->setVersion(0x11);
    res->setStatus(HttpStatus::OK);
    res->setBody("this is index page");
}
