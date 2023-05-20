#include "HttpServer.h"
#include "HttpConnection.h"
#include "HttpSession.h"
#include "../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

HttpServer::HttpServer(int threadCount): TcpServer(threadCount)
{
}

void HttpServer::onConnect()
{
	SockAddr clientAddr;
	SOCKET sock = m_sock->accept(clientAddr);
	if(sock == SOCKET_ERROR)
	{
		return;
	}
	
	std::shared_ptr<EventLoop> loop = m_pool->getNextLoop();

	m_sessionId++;
	std::shared_ptr<HttpConnection> connection(new HttpConnection(sock, loop));
	loop->addSession(std::make_shared<HttpSession>(m_sessionId, connection, loop));
}

bool HttpServer::init(std::string ip, int port)
{
	m_pool->start();

	if(!bind(ip, port) || !listen())
	{
		LOG_FATAL(g_logger) << "server init failed";
		return false;
	}

	m_channel->setReadCallback(std::bind(&HttpServer::onConnect, this));
	m_channel->setErrorCallback(std::bind(&TcpServer::onError, this));

	return true;
}
