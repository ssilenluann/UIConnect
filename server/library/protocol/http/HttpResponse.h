#ifndef PROTOCOL_HTTP_HTTPRESPONSE_H
#define PROTOCOL_HTTP_HTTPRESPONSE_H

#include <memory>
#include <string>
#include <iostream>
#include <map>

#include "Http.h"

class HttpResponse
{
public:
    typedef std::shared_ptr<HttpResponse> ptr;
    typedef std::map<std::string, std::string, CaseInsensitive> MapType;
    HttpResponse(uint8_t version = 0x11, bool close = true);

    inline HttpStatus getStatus() const { return m_status;}
    inline uint8_t getVersion() const { return m_version;}
    inline const std::string& getBody() const { return m_body;}
    inline const std::string& getReason() const { return m_reason;}
    inline const MapType& getHeaders() const { return m_headers;}

    inline void setStatus(HttpStatus v) { m_status = v;}
    inline void setVersion(uint8_t v) { m_version = v;}
    inline void setBody(const std::string& v) { m_body = v;}
    inline void setReason(const std::string& v) { m_reason = v;}
    inline void setHeaders(const MapType& v) { m_headers = v;}

    bool isClose() const {return m_close;}
    bool setClose(bool v = false) {m_close = v;}

    std::string getHeader(const std::string& key, const std::string& defaultVal) const;
    void setHeader(const std::string& key, const std::string& val);
    void delHeader(const std::string& key);

    template<class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& defaultVal = T())
    {
        return checkGetAs(m_headers, key, defaultVal);
    }

    template<class T>
    T getHeaderAs(const std::string& key, const T& defaultVal = T())
    {
        return getAs(m_headers, key, defaultVal);
    }

    template<class T>
    bool checkGetParamAs(const std::string& key, T& val, const T& defaultVal = T())
    {
        return checkGetAs(m_headers, key, defaultVal);
    }

    template<class T>
    T getParamAs(const std::string& key, const T& defaultVal = T())
    {
        return getAs(m_headers, key, defaultVal);
    }

    template<class T>
    bool checkGetCookieAs(const std::string& key, T& val, const T& defaultVal = T())
    {
        return checkGetAs(m_headers, key, defaultVal);
    }

    template<class T>
    T getCookieAs(const std::string& key, const T& defaultVal = T())
    {
        return getAs(m_headers, key, defaultVal);
    }
    std::ostream& dump(std::ostream& ost);

private:
    HttpStatus m_status;
    uint8_t m_version;
    bool m_close;
    bool m_websocket;
    std::string m_body;
    std::string m_reason;
    MapType m_headers;

};
#endif