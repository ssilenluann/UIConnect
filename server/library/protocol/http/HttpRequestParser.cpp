#ifndef PROTOCOL_HTTP_HTTPREQUESTPARSER_CPP
#define PROTOCOL_HTTP_HTTPREQUESTPARSER_CPP

#include "HttpRequestParser.h"
#include "Http.h"

static Logger::ptr g_logger = LOG_NAME("system");

static ConfigItem<uint64_t>::ptr g_http_request_buffer_size = 
    Config::SearchOrAdd("http.request.buffer_size", (uint64_t)4*1024, "http request size");

static ConfigItem<uint64_t>::ptr g_http_request_max_body_size = 
    Config::SearchOrAdd("http.request.max_body_size", (uint64_t)64*1024, "max http request body size");

static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;

struct _RequestSizeIniter
{
    _RequestSizeIniter()
    {
        s_http_request_buffer_size = g_http_request_buffer_size->getValue();
        s_http_request_max_body_size = g_http_request_max_body_size->getValue();

        g_http_request_buffer_size->addListener([](const uint64_t& oldValue, const uint64_t& newValue)
        {
            s_http_request_buffer_size = newValue;
        });

        g_http_request_max_body_size->addListener([](const uint64_t& oldValue, const uint64_t& newValue)
        {
            s_http_request_max_body_size = newValue;
        });
    }
};

static _RequestSizeIniter _httpRequestSizeIniter;

void on_request_method(void* data, const char* at, size_t length)
{
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    HttpMethod m = Http::String2HttpMethod(at);

    if(m == HttpMethod::INVALID_METHOD)
    {
        LOG_WARN(g_logger) << "invalid http request method: " << std::string(at, length);
        parser->setError(1000);
        return;
    }

    parser->getData()->setMethod(m);
}

void on_request_uri(void* data, const char* at, size_t length)
{
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
}

void on_request_fragment(void* data, const char* at, size_t length)
{

    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setFragment(std::string(at, length));
}

void on_request_path(void* data, const char* at, size_t length)
{
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setPath(std::string(at, length));

}

void on_request_query(void* data, const char* at, size_t length)
{
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setQuery(std::string(at, length));

}

void on_request_version(void* data, const char* at, size_t length)
{
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    uint8_t v = 0;
    if(strncmp(at, "HTTP/1.1", length) == 0)
    {
        v = 0x11;
    }
    else if(strncmp(at, "HTTP/1.0", length) == 0)
    {
        v = 0x10;
    }
    else
    {
        LOG_WARN(g_logger) << "invalid http request version: " << std::string(at, length);
        parser->setError(-1001);
        return;
    }

    parser->getData()->setVersion(v);
}

void on_request_header_done(void* data, const char* at, size_t length)
{
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
}


void on_request_http_field(void* data, const char* field, size_t flen, const char* value, size_t vlen)
{
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);

    if(flen == 0)
    {
        LOG_WARN(g_logger) << "invalid http request field length: 0";
        parser->setError(1002);
        return;
    }

    parser->getData()->setHeader(std::string(field, flen), std::string(value, vlen));
}

HttpRequestParser::HttpRequestParser(): m_error(0)
{
    m_data.reset(new HttpRequest());
    http_parser_init(&m_parser);
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query;
    m_parser.http_version = on_request_version;
    m_parser.header_done = on_request_header_done;
    m_parser.http_field = on_request_http_field;
    m_parser.data = this;
}

int HttpRequestParser::isFinished()
{
    return http_parser_finish(&m_parser);
}

int HttpRequestParser::hasError()
{
    return m_error || http_parser_has_error(&m_parser);
}

uint64_t HttpRequestParser::getContentLength()
{
    return m_data->getHeaderAs<uint64_t>("content-length", 0);
}

size_t HttpRequestParser::execute(char *data, size_t len)
{
    size_t offset = http_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + offset, len - offset);
    return offset;
}


#endif