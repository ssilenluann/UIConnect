#ifndef PROTOCOL_HTTP_HTTPSESSION_CPP
#define PROTOCOL_HTTP_HTTPSESSION_CPP

#include "HttpSession.h"
#include "HttpRequestParser.h"
#include <sstream>
#include <string>

HttpSession::HttpSession(Socket::ptr sock) : SocketStream(sock)
{
}

HttpRequest::ptr HttpSession::recvRequest()
{
    HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
    uint64_t buffer_size = HttpRequestParser::GetHttpRequestBufferSize();

    // apply memory from heap instead of stack
    std::shared_ptr<char> buffer(
        new char[buffer_size], [](char* ptr)
        {
            delete[] ptr;
        }
    );

    char* data = buffer.get();
    int offset = 0;
    do
    {
        int len = read(data + offset, buff_size - offset);
        if(len <= 0)
        {
            close();
            return nullptr;
        }

        len += offset;
        size_t nParsed = parser->execute(data, len);
        if(parser->hasError())
        {
            close();
            return nullptr;
        }

        offset = len - nParsed;
        if(offset == (int)buff_size)
        {
            close();
            return nullptr;
        }
        
    } while(parser->isFinished());
}

int HttpSession::sendResponse(HttpResponse::ptr rsp)
{
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return wirteFixedSize(data.c_str(), data.size());
}

#endif