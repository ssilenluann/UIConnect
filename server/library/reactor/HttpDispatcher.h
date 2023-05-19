#ifndef REACTOR_DISPATCHER_H
#define REACTOR_DISPATCHER_H
#include "../protocol/http/HttpRequest.h"
#include "../protocol/http/HttpResponse.h"

#include "HttpProcessor.h"
class HttpDispatcher
{
private:
    HttpDispatcher() = default;
    HttpDispatcher(const HttpDispatcher& hd) = delete;
    const HttpDispatcher& operator=(const HttpDispatcher& rhs) = delete;

public:
    static void Dispatch(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> res);
    static void Regist(const std::string& str, std::shared_ptr<HttpProcessor> hp);
    static HttpDispatcher& Instance();

private:
    static std::map<std::string, std::shared_ptr<HttpProcessor>> m_processors;
};
#endif