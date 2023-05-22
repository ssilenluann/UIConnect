#ifndef REACTOR_DISPATCHER_H
#define REACTOR_DISPATCHER_H
#include "../protocol/http/HttpRequest.h"
#include "../protocol/http/HttpResponse.h"

#include "HttpProcessor.h"
#include <unordered_map>
#include <list>
#include <map>
#include <vector>
#include <boost/regex.hpp>

class HttpDispatcher
{
private:
    HttpDispatcher() = default;
    HttpDispatcher(const HttpDispatcher& hd) = delete;
    const HttpDispatcher& operator=(const HttpDispatcher& rhs) = delete;

public:
    void dispatch(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> res);
    void regist(const std::string& str, std::shared_ptr<HttpProcessor> hp, bool supportFuzzyMatch = false);
    void fuzzyRegist(const std::string& str, std::shared_ptr<HttpProcessor> hp);

    static HttpDispatcher& Instance();

private:
    static std::unordered_map<std::string, std::shared_ptr<HttpProcessor>> m_processors;
    static std::map<std::string, std::pair<boost::regex, std::shared_ptr<HttpProcessor>>> m_fuzzyProcessors;
};
#endif