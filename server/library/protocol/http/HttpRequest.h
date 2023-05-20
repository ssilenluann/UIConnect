#ifndef PROTOCOL_HTTP_HTTPREQUEST_H
#define PROTOCOL_HTTP_HTTPREQUEST_H

#include <stdint.h>
#include <string>
#include <map>
#include <memory>
#include <boost/lexical_cast.hpp>

#include "Http.h"

class HttpRequest
{
public:
    typedef std::shared_ptr<HttpRequest> ptr;
    typedef std::map<std::string, std::string, CaseInsensitive> MapType;
    HttpRequest(uint8_t version = 0x11, bool close = true);
    
    inline HttpMethod getMethod() const {return m_method;}
    inline HttpStatus getStatus() const {return m_status;}
    inline uint8_t getVersion() const {return m_version;}
    inline const std::string& getPath() const {return m_path;}
    inline const std::string& getQuery() const {return m_query;}
    inline const std::string& getFragment() const {return m_fragment;}
    inline const std::string& getBody() const {return m_body;}
    inline const MapType& getHeaders() const {return m_headers;}
    inline const MapType& getParams() const {return m_params;}
    inline const MapType& getCookies() const {return m_cookies;}

    inline void setMethod(HttpMethod v) { m_method = v;}
    inline void setStatus(HttpStatus v) { m_status = v;}
    inline void setVersion(uint8_t v) { m_version = v;}
    inline void setPath(const std::string& v) { m_path = v;}
    inline void setQuery(const std::string& v) { m_query = v;}
    inline void setFragment(const std::string& v) { m_fragment = v;}
    inline void setBody(const std::string& v) { m_body = v;}
    inline void setHeaders(const MapType& v) { m_headers = v;}
    inline void setParams(const MapType& v) { m_params = v;}
    inline void setCookies(const MapType& v) { m_cookies = v;}

    std::string getHeader(const std::string& key, const std::string& defaultVal = "");
    std::string getParam(const std::string& key, const std::string& defaultVal = "");
    std::string getCookie(const std::string& key, const std::string& defaultVal = "");

    void setHeader(const std::string& key, const std::string& val);
    void setParam(const std::string& key, const std::string& val);
    void setCookie(const std::string& key, const std::string& val);

    void delHeader(const std::string& key);
    void delParam(const std::string& key);
    void delCookie(const std::string& key);

    bool hasHeader(const std::string& key, std::string* val = nullptr);
    bool hasParam(const std::string& key, std::string* val = nullptr);
    bool hasCookie(const std::string& key, std::string* val = nullptr);

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
    HttpMethod m_method;
    HttpStatus m_status;
    uint8_t m_version;
    bool m_close;
    bool m_websocket;

    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    std::string m_body;

    MapType m_headers;
    MapType m_params;
    MapType m_cookies;
};
#endif