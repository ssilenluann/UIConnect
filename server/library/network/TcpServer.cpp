
#include <iostream>
#include "TcpServer.h"
#include "../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

TcpServer::TcpServer(int threadCount)
: m_loop(new EventLoop()), 
m_sock(new TcpSocket()), 
m_channel(new TcpChannel(m_loop, m_sock->fd())),
m_pool(new EventThreadPool(threadCount))
{
}

TcpServer::~TcpServer(){}

bool TcpServer::bind(std::string ip, int port)
{
	int retp =  m_sock->bind(ip, port);
	if(retp < 0)
	{
		LOG_FMT_FATAL(g_logger, "server bind port error, socket fd = %d, errno = %d", m_sock->fd(), errno);
	}

	return retp >= 0;
}

bool TcpServer::listen()
{
	m_sock->listen();
	bool retp = m_channel->enableReading();
	if(!retp)
	{
		LOG_FMT_FATAL(g_logger, "server listen port error, socket fd = %d, errno = %d", m_sock->fd(), errno);
	}

	return retp;
}

bool TcpServer::init(std::string ip, int port)
{
	m_pool->start();

	if(!bind(ip, port) || !listen())
	{
		LOG_FATAL(g_logger) << "server init failed";
		return false;
	}

	m_channel->setReadCallback(std::bind(&TcpServer::onConnect, this));
	m_channel->setErrorCallback(std::bind(&TcpServer::onError, this));

	return true;
}

void TcpServer::run()
{
	LOG_INFO(g_logger) << "server started";
	m_loop->loop();
	LOG_INFO(g_logger) << "server main loop ended";
}

void TcpServer::onConnect()
{
	SockAddr clientAddr;
	SOCKET sock = m_sock->accept(clientAddr);
	if(sock == SOCKET_ERROR)
	{
		return;
	}
	
	std::shared_ptr<EventLoop> loop = m_pool->getNextLoop();

	m_sessionId++;
	std::unique_ptr<TcpConnection> connection(new TcpConnection(sock, loop));
	loop->addSession(std::make_shared<TcpSession>(m_sessionId, std::move(connection), loop));

}


void TcpServer::onError()
{
	std::cout << m_sock->fd() << " error, msg: " << m_sock->getLastError() << std::endl;
}

void TcpServer::exit()
{
	LOG_INFO(g_logger) << "server start quit";
	// 1. disable listen channel
	m_channel->disable();
	// 2. quit listen loop
	m_loop->quit();	
	// 3. close thread pool
	m_pool->quit();
	// 4.close listen sock
	m_sock->close();
	LOG_INFO(g_logger) << "server quited";
}

void TcpServer::setStartCallback(CALLBACK func)
{
	m_startCallback = func;
}

void TcpServer::setErrorCallback(CALLBACK func)
{
	m_errorCallback = func;
}

void TcpServer::setQuitCallback(CALLBACK func)
{
	m_quitCallback = func;
}
