#ifndef NETWORK_TCPSERVER_H
#define NETWORK_TCPSERVER_H

#include <memory>
#include <vector>
#include "./socket/TcpSocket.h"
#include "./event/EventLoop.h"
#include "./event/EventThreadPool.h"
#include "./TcpConnection.h"

class TcpServer 
{
public:
	TcpServer();
	~TcpServer();
	bool bind(std::string ip, int port);
	bool listen();
	bool init(std::string ip, int port);
	void run();
	void onConnect();
	

private:
	std::shared_ptr<TcpSocket> m_sock;
	std::shared_ptr<EventLoop> m_loop;
	std::shared_ptr<TcpChannel> m_channel;
	std::vector<std::unique_ptr<TcpConnection>> m_connections;
	EventThreadPool m_pool;

};

#endif
