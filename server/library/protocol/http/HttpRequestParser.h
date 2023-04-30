#ifndef PROTOCOL_HTTP_HTTPREQUESTPARSER_H
#define PROTOCOL_HTTP_HTTPREQUESTPARSER_H

#include <memory>
#include <http11/http11_parser.h>
#include <stdint.h>

#include "HttpRequest.h"
#include "../../log/Logger.h"
#include "../../config/Config.h"

class HttpRequestParser
{
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    HttpRequestParser();

    int isFinished();
    int hasError();
    /**
     * @return:
     *  1: succeed
     *  -1: error
     *  >0: byte count that processed
    */
    size_t execute(char* data, size_t len);

    HttpRequest::ptr getData() { return m_data;}
    void setError(int v) { m_error = v;};
    uint64_t getContentLength();
private:
    http_parser m_parser;
    HttpRequest::ptr m_data;
    // 1000: invalid request method
    // 1001: invalid request version
    // 1002: invalid request field length
    int m_error;        
};
#endif