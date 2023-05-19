#include "HttpDispatcher.h"

std::map<std::string, std::shared_ptr<HttpProcessor>> HttpDispatcher::m_processors;

void HttpDispatcher::Dispatch(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> res)
{
    std::string query = req->getPath();
    if(m_processors.find(query) == m_processors.end())
    {
        res->setStatus(HttpStatus::NOT_FOUND);
        res->setBody("resouce not found");
        return;
    }

    m_processors[query]->handleHttpRequest(req, res);
}

void HttpDispatcher::Regist(const std::string & str, std::shared_ptr<HttpProcessor> hp)
{
    m_processors[str] = hp;
}

HttpDispatcher & HttpDispatcher::Instance()
{
    static HttpDispatcher instance;
    return instance;
}
