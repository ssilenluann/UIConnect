#include "HttpDispatcher.h"
#include "../utils/StringUtil.h"
#include "../log/Logger.h"

#include <boost/regex.hpp>
#include <sstream>

std::unordered_map<std::string, std::shared_ptr<HttpProcessor>> HttpDispatcher::m_processors;
std::map<std::string, std::pair<boost::regex, std::shared_ptr<HttpProcessor>>> HttpDispatcher::m_fuzzyProcessors;

static Logger::ptr g_logger = LOG_NAME("system");

void HttpDispatcher::dispatch(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> res)
{
    std::string query = req->getPath();

    if(m_processors.find(query) != m_processors.end())
    {
        m_processors[query]->handleHttpRequest(req, res);
        return;
    }

    // fuzzy match
    for(auto& fp: m_fuzzyProcessors)
    {
        if(boost::regex_match(query, fp.second.first) )
        {
            LOG_DEBUG(g_logger) << "path: "<< query << ", match pattern: " << fp.second.first;
            fp.second.second->handleHttpRequest(req, res);
            return;
        }
    }

    // 404 not found
    res->setVersion(0x11);
    res->setHeader("Server", "Luansi/UIConnect");
    res->setClose(false);
    res->setStatus(HttpStatus::NOT_FOUND);
    res->setBody("resouce not found");
    return;
}

void HttpDispatcher::regist(const std::string& str, std::shared_ptr<HttpProcessor> hp, bool supportFuzzyMatch)
{
    m_processors[str] = hp;

    if(supportFuzzyMatch)
        fuzzyRegist(str, hp);
}

void HttpDispatcher::fuzzyRegist(const std::string & str, std::shared_ptr<HttpProcessor> hp)
{
    if(m_fuzzyProcessors.find(str) != m_fuzzyProcessors.end())
        return;

    std::ostringstream oss;
    oss << "(^" << str << "/+.*)|(" << str << ")";
    m_fuzzyProcessors[str] = {boost::regex(oss.str(), boost::regex_constants::icase), hp};
}

HttpDispatcher & HttpDispatcher::Instance()
{
    static HttpDispatcher instance;
    return instance;
}
