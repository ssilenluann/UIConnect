#ifndef NETWORK_TCPSERVER_CPP
#define NETWORK_TCPSERVER_CPP

#include "TcpServer.h"

TcpServer::TcpServer(): m_sock(new TcpSocket()), m_loop(new EventLoop(m_sock->fd())), m_channel(new TcpChannel(m_loop, m_sock->fd()))
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
	bool retp = m_channel->enableReading();
	if(!retp)
	{
		// TODO: LOG, listen failed
	}

	return retp;
}

bool TcpServer::init(std::string ip, int port)
{
	if(!bind(ip, port) || !listen())
	{
		// TODO: LOG
		return false;
	}

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
	if(socket == SOCKET_ERROR)
	{
		// TODO: LOG
	}
	
	std::shared_ptr<EventLoop> loop = m_pool.getNextLoop();
	auto connection = std::unique_ptr<TcpConnection>(new TcpConnection(sock, loop));
	bool retp = connection->init();
	if(!retp)
	{
		//TODO: LOG
		connection.reset();
		return;
	}
	m_connections.push_back(connection);
}	
#endif
