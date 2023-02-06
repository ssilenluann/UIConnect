#ifndef NETWORK_TCPSERVER_CPP
#define NETWORK_TCPSERVER_CPP

#include <iostream>
#include "TcpServer.h"

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
		// TODO: LOG, bind failed
	}

	return retp >= 0;
}

bool TcpServer::listen()
{
	m_sock->listen();
	bool retp = m_channel->enableReading();
	if(!retp)
	{
		// TODO: LOG, listen failed
	}

	return retp;
}

bool TcpServer::init(std::string ip, int port)
{
	m_pool->start();

	if(!bind(ip, port) || !listen())
	{
		// TODO: LOG
		return false;
	}

	m_channel->setReadCallback(std::bind(&TcpServer::onConnect, this));
	m_channel->setErrorCallback(std::bind(&TcpServer::onError, this));

	return true;
}

void TcpServer::run()
{
	m_loop->loop();
}

void TcpServer::onConnect()
{
	SockAddr clientAddr;
	SOCKET sock = m_sock->accept(clientAddr);
	if(sock == SOCKET_ERROR)
	{
		// TODO: LOG
	}
	
	std::shared_ptr<EventLoop> loop = m_pool->getNextLoop();

	m_sessionId++;
	std::unique_ptr<TcpConnection> connection(new TcpConnection(sock, loop));
	auto session = std::unique_ptr<TcpSession>(new TcpSession(m_sessionId, std::move(connection), loop));
	loop->addSession(std::move(session));

}


void TcpServer::onError()
{
	// TODO: LOG
	std::cout << m_sock->fd() << " error, msg: " << m_sock->getLastError() << std::endl;
}

void TcpServer::exit()
{
	// TODO: LOG
	// 1. disable listen channel
	m_channel->disable();
	// 2. quit listen loop
	m_loop->quit();	
	// 3. close thread pool
	m_pool->quit();
	// 4.close listen sock
	m_sock->close();
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
#endif
