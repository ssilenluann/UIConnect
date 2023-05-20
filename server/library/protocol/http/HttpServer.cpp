#include "HttpServer.h"
#include "HttpConnection.h"
#include "HttpSession.h"

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
	bool retp = TcpServer::init(ip, port);
	m_channel->setReadCallback(std::bind(&HttpServer::onConnect, this));

	return retp;
}
