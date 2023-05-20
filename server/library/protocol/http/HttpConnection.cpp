#include "HttpConnection.h"
#include "Http.h"
#include "HttpRequestParser.h"
#include "HttpResponseParser.h"

HttpConnection::HttpConnection(SOCKET fd, std::shared_ptr<EventLoop> loop): TcpConnection(fd, loop)
{
}

void HttpConnection::onRead()
{
    if(m_state != ConnState::Connected) return;

    int size = 0;
    bool retp = m_socket->recv(m_readBuffer, size);
    
    // close
    if(retp == false && size == 0)
    {
        onClose();
        return ;
    }

    // error
    if(retp == false && size == -1)
    {
        // TODO: LOG
        onError();
        return;
    }

    for(;;)
    {
        if(m_readBuffer->pos() == 0 || m_readBuffer->find(" ", 1) == std::string::npos)    break;
        
        std::string methodOrRes(m_readBuffer->start(), m_readBuffer->find(" ", 1));
        if(Http::String2HttpMethod(methodOrRes) != HttpMethod::INVALID_METHOD)
        {
            // get http request
            HttpRequestParser reqParser;
            
            int parsedSize = reqParser.execute(m_readBuffer->start(), m_readBuffer->pos());
            if(reqParser.isFinished())
            {
                // get a complete request
                m_readBuffer->remove(parsedSize);
                if(m_readCallback)
                    m_readCallback(reqParser.getData());                
                continue;
            }
            if(reqParser.hasError() > 0)
            {
                // TODO: error
            }
            // not complete
            break;
        }

        if(methodOrRes.find("HTTP/1.1") != std::string::npos || 
            methodOrRes.find("HTTP/1.0") != std::string::npos)
        {
            // get http response
            HttpResponseParser resParser;
            
            int parsedSize = resParser.execute(m_readBuffer->start(), m_readBuffer->pos());
            if(resParser.isFinished() == 1)
            {
                // get a complete response
                m_readBuffer->remove(parsedSize);
                // TODO:
                continue;
            }
            if(resParser.hasError() > 0)
            {
                // TODO: error
            }
            // not complete
            break;
        }

        // TODO: wrong http request/response
    }

    
}

bool HttpConnection::init()
{
    m_channel->setReadCallback(std::bind(&HttpConnection::onRead, this));
    m_channel->setWriteCallback(std::bind(&HttpConnection::onWrite, this));
    m_channel->setErrorCallback(std::bind(&TcpConnection::onError, this));
    m_channel->setCloseCallback(std::bind(&TcpConnection::onClose, this));
	
	bool retp =  m_channel->addTargetEvent(TcpChannel::readEvent);
    if(retp)
        m_state = ConnState::Connected;

    return retp;
}

void HttpConnection::setReadCallback(HTTP_READ_CB func)
{
    m_readCallback = func;
}

bool HttpConnection::send(std::shared_ptr<HttpResponse> &res)
{
    std::stringstream oss;
    res->dump(oss);
    m_writeBuffer->setMsg(oss.str().c_str(), oss.str().size());
    return m_channel->enableWriting();
}
