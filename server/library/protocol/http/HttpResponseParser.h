#ifndef PROTOCOL_HTTP_HTTPRESPONSEPARSER_H
#define PROTOCOL_HTTP_HTTPRESPONSEPARSER_H

#include <http11/httpclient_parser.h>

#include "HttpResponse.h"
class HttpResponseParser
{
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser();

    int isFinished();
    int hasError();
    size_t execute(char* data, size_t len);

    HttpResponse::ptr getData() { return m_data;}
    void setError(int v) { m_error = v;};
    uint64_t getContentLength();
private:
    httpclient_parser m_parser;
    HttpResponse::ptr m_data;
    // 1001: invalid response version
    // 1002: invalid response field length
    int m_error;      
};


#endif

