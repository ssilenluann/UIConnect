#include "HttpConnection.h"
#include "Http.h"
#include "HttpRequestParser.h"
#include "HttpResponseParser.h"
#include "../../network/event/EventLoop.h"
#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");
HttpConnection::HttpConnection(SOCKET fd, std::shared_ptr<EventLoop> loop): m_incompleteReqCnt(0), TcpConnection(fd, loop)
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
        onError();
        return;
    }
    m_incompleteReqCnt++;
    for(;;)
    {
        std::string methodOrRes;
        if(m_readBuffer->getUnreadSize() == 0 
            || m_readBuffer->find(" ", methodOrRes, 20) == std::string::npos)    break;

        if(Http::String2HttpMethod(methodOrRes) != HttpMethod::INVALID_METHOD)
        {
            // get http request
            HttpRequestParser reqParser;
            std::string msg = m_readBuffer->toString();
            int parsedSize = reqParser.execute(const_cast<char*>(msg.c_str()), msg.size());
            if(reqParser.isFinished() == 1)
            {
                // get a complete request
                m_incompleteReqCnt = 0;
                m_readBuffer->hasRead(parsedSize);
                reqParser.getData()->setSize(parsedSize);
                if(m_readCallback)
                    m_readCallback(reqParser.getData());                
                continue;
            }
            if(reqParser.hasError() > 0)
            {
                onParseRequestError(msg);
            }
            // not complete
            break;
        }

        if(methodOrRes.find("HTTP/1.1") != std::string::npos || 
            methodOrRes.find("HTTP/1.0") != std::string::npos)
        {
            // get http response
            HttpResponseParser resParser;
            std::string msg = m_readBuffer->toString();
            int parsedSize = resParser.execute(const_cast<char*>(msg.c_str()), msg.size());
            if(resParser.isFinished() == 1)
            {
                // get a complete response
                m_incompleteReqCnt = 0;
                m_readBuffer->hasRead(parsedSize);
                // TODO:
                continue;
            }
            if(resParser.hasError() > 0)
            {
                onParseResponseError(msg);
            }
            // not complete
            break;
        }

        // wrong http request/response
        m_loop.lock()->addTask([&]()
        {
            onClose();
        });
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

void HttpConnection::onParseRequestError(std::string & msg)
{
    std::shared_ptr<HttpResponse> res(new HttpResponse());
    res->setVersion(0x11);
    res->setHeader("Server", "Luansi/UIConnect");
    res->setStatus(HttpStatus::BAD_REQUEST);
    res->setClose(false);
    res->setBody("bad request: \r\n" + msg);
    
    send(res);

    m_loop.lock()->addTask([&]()
    {
        onClose();
    });
}

void HttpConnection::onParseResponseError(std::string & msg)
{
    LOG_DEBUG(g_logger) << "parse res error: \r\n" << msg;

    m_loop.lock()->addTask([&]()
    {
        onClose();
    });
}

void HttpConnection::resetIncompReqCnt()
{
    m_incompleteReqCnt = 0;
}

int HttpConnection::getIncompReqCnt()
{
    return m_incompleteReqCnt;
}
