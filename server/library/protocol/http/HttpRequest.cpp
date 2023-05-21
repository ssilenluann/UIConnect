#ifndef PROTOCOL_HTTP_HTTPREQUEST_CPP
#define PROTOCOL_HTTP_HTTPREQUEST_CPP

#include "HttpRequest.h"
#include "Http.h"

#include <strings.h>
bool CaseInsensitive::operator()(const std::string & lhs, const std::string & rhs) const
{
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HttpRequest::HttpRequest(uint8_t version, bool close)    
    : m_method(HttpMethod::GET), m_version(version), m_close(close), m_websocket(false), m_path("/"), m_size(0)
{
}

std::string HttpRequest::getHeader(const std::string &key, const std::string &defaultVal)
{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? defaultVal : it->second;
}

std::string HttpRequest::getParam(const std::string &key, const std::string &defaultVal)
{
    
    auto it = m_params.find(key);
    return it == m_params.end() ? defaultVal : it->second;
}

std::string HttpRequest::getCookie(const std::string &key, const std::string &defaultVal)
{
    auto it = m_cookies.find(key);
    return it == m_cookies.end() ? defaultVal : it->second;
}

void HttpRequest::setHeader(const std::string &key, const std::string &val)
{
    m_headers[key] = val;
}

void HttpRequest::setParam(const std::string &key, const std::string &val)
{
    m_params[key] = val;
}

void HttpRequest::setCookie(const std::string &key, const std::string &val)
{
    m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string &key)
{
    m_headers.erase(key);
}

void HttpRequest::delParam(const std::string &key)
{
    m_params.erase(key);
}

void HttpRequest::delCookie(const std::string &key)
{
    m_cookies.erase(key);
}

bool HttpRequest::hasHeader(const std::string &key, std::string *val)
{
    auto it = m_headers.find(key);
    if(it == m_headers.end())   return false;
    if(val) *val = it->second;
    return true;
}

bool HttpRequest::hasParam(const std::string &key, std::string *val)
{
    auto it = m_params.find(key);
    if(it == m_params.end())   return false;
    if(val) *val = it->second;
    return true;
}

bool HttpRequest::hasCookie(const std::string &key, std::string *val)
{
    auto it = m_cookies.find(key);
    if(it == m_cookies.end())   return false;
    if(val) *val = it->second;
    return true;
}

std::ostream &HttpRequest::dump(std::ostream &ost)
{
    //GET /uri HTTP/1.1
    //Host:: www.baidu.com
    //
    //
    ost << Http::HttpMethod2String(m_method) << " "
        << m_path
        << (m_query.empty() ? "" : "?")
        << m_query
        << (m_fragment.empty() ? "" : "#")
        << m_fragment
        << " HTTP/" 
        << ((uint32_t)(m_version >> 4))
        << "."
        << ((uint32_t)(m_version) & 0x0F)
        << "\r\n"
        << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";

    for(auto& i: m_headers)
    {
        if(strcasecmp(i.first.c_str(), "connection") == 0)
            continue;
        ost << i.first << ":" << i.second << "\r\n";
    }

    if(!m_body.empty())
    {
        ost << "content-length: " << m_body.size() << "\r\n\r\n"
            << m_body;
    }
    else
    {
        ost << "\r\n" << m_body;
    }

    return ost;
}
#endif