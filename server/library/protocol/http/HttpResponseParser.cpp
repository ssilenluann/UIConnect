#ifndef PROTOCOL_HTTP_HTTPRESPONSEPARSER_CPP
#define PROTOCOL_HTTP_HTTPRESPONSEPARSER_CPP

#include "HttpResponseParser.h"
#include "../../log/Logger.h"
#include "Http.h"

#include <string.h>

static Logger::ptr g_logger = LOG_ROOT();
void on_response_reason(void* data, const char* at, size_t length)
{
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getData()->setReason(std::string(at, length));
}

void on_response_status(void* data, const char* at, size_t length)
{
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getData()->setStatus((HttpStatus)atoi(at));
}

void on_response_chunk(void* data, const char* at, size_t length)
{
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    
}

void on_response_version(void* data, const char* at, size_t length)
{
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    uint8_t v = 0;
    v = (strncmp(at, "HTTP/1.1", length) == 0) ? 0x11 : v;
    v = (strncmp(at, "HTTP/1.0", length) == 0) ? 0x10 : v;
    if(v == 0)
    {
        LOG_WARN(g_logger) << "invalid http response version: " << std::string(at, length);
        parser->setError(-1001);
        return;
    }

    parser->getData()->setVersion(v);
}

void on_response_header_done(void* data, const char* at, size_t length)
{
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    
}

void on_response_last_chunk(void* data, const char* at, size_t length)
{
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    
}

void on_response_http_field(void* data, const char* field, size_t flen, const char* value, size_t vlen)
{
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    if(flen == 0)
    {
        LOG_WARN(g_logger) << "invalid http response field length: 0";
        parser->setError(1002);
        return;
    }

    parser->getData()->setHeader(std::string(field, flen), std::string(value, vlen));
 
}

HttpResponseParser::HttpResponseParser(): m_error(0)
{
    m_data.reset(new HttpResponse());
    httpclient_parser_init(&m_parser);
    m_parser.reason_phrase = on_response_reason;
    m_parser.status_code = on_response_status;
    m_parser.chunk_size = on_response_chunk;
    m_parser.http_version = on_response_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;
    m_parser.data = this;
}

int HttpResponseParser::isFinished()
{
    return httpclient_parser_finish(&m_parser);
}

int HttpResponseParser::hasError()
{
    return m_error || httpclient_parser_has_error(&m_parser);
}

size_t HttpResponseParser::execute(char *data, size_t len)
{
    size_t offset = httpclient_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + offset, len - offset);
    return offset;
}
uint64_t HttpResponseParser::getContentLength()
{
    return m_data->getHeaderAs<uint64_t>("content-length", 0);
}
#endif