#ifndef NETWORK_TCPSERVER_CPP
#define NETWORK_TCPSERVER_CPP

#include <iostream>
#include "TcpServer.h"

TcpServer::TcpServer(int threadCount)
: m_sock(new TcpSocket()), 
m_loop(new EventLoop(m_sock->fd())), 
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
	if(sock == SOCKET_ERROR)
	{
		// TODO: LOG
	}
	
	std::shared_ptr<EventLoop> loop = m_pool->getNextLoop();
	auto connection = std::unique_ptr<TcpConnection>(new TcpConnection(sock, loop));
	bool retp = connection->init();
	if(!retp)
	{
		//TODO: LOG
		connection.reset();
		return;
	}

	connection->setReadCallback(std::bind(&TcpServer::onRead, this, std::placeholders::_1));
	connection->setWriteCallback(std::bind(&TcpServer::onWrite, this, std::placeholders::_1));
	connection->setErrorCallback(std::bind(&TcpServer::onError, this, std::placeholders::_1));
	connection->setCloseCallback(std::bind(&TcpServer::onClose, this, std::placeholders::_1));
	m_connections.emplace(sock, std::move(connection));

	// TODO: LOG
}

void TcpServer::onRead(Packet& packet)
{
	std::cout << packet.dataLoad() << std::endl;
}

void TcpServer::onWrite(int size)
{

}

void TcpServer::onError(SOCKET socket)
{
	// TODO: LOG
	std::cout << socket << " error \r\n";
	onClose(socket); 
}

void TcpServer::onClose(SOCKET socket)
{
	// TODO: LOG
	std::cout << "connection of socket " << socket << " will be closed son\r\n";
	std::lock_guard<std::mutex> lock(m_mutex);
	m_disconnectingSock.push_back(socket);
}

void TcpServer::closeConnection()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	std::map<SOCKET, std::unique_ptr<TcpConnection>>::iterator it;
	for(auto fd: m_disconnectingSock)
	{
		it = m_connections.find(fd);
		if(it == m_connections.end())
		{
			// TODO: LOG
			continue;
		}
		if(!m_connections[fd]->isClosed())
			continue;
		
		m_connections.erase(it);
	}
}

#endif
