#ifndef PROTOCOL_HTTP_HTTPRESPONSE_CPP
#define PROTOCOL_HTTP_HTTPRESPONSE_CPP

#include "HttpResponse.h"
#include <string.h>

HttpResponse::HttpResponse(uint8_t version, bool close)
    : m_status(HttpStatus::OK), m_version(version), m_close(close)
{
}

std::string HttpResponse::getHeader(const std::string &key, const std::string &defaultVal) const
{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? defaultVal : it->second;
}

void HttpResponse::setHeader(const std::string &key, const std::string &val)
{
    m_headers[key] = val;
}

void HttpResponse::delHeader(const std::string &key)
{
    m_headers.erase(key);
}

std::ostream &HttpResponse::dump(std::ostream &ost)
{
    ost << "HTTP/"
        << ((uint32_t)(m_version >> 4))
        << "."
        << ((uint32_t)(m_version & 0x0F))
        << " "
        << (uint32_t)m_status
        << " "
        << (m_reason.empty()? Http::HttpStatus2String(m_status): m_reason)
        << "\r\n";

    for(auto&i : m_headers)
    {
        if(strcasecmp(i.first.c_str(), "connection") == 0)
            continue;
        
        ost << i.first << ": " << i.second << "\r\n";
    }

    ost << "connection: " << (m_close? "close": "keep-alive") << "\r\n";

    if(!m_body.empty())
        ost << "content-length: " << m_body.size() << "\r\n\r\n";
    else
        ost << "\r\n";
    
    return ost;
}

#endif