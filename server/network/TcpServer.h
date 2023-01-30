#ifndef NETWORK_TCPSERVER_H
#define NETWORK_TCPSERVER_H

#include <memory>
#include <vector>
#include <list>
#include "./socket/TcpSocket.h"
#include "./event/EventLoop.h"
#include "./event/EventThreadPool.h"
#include "./TcpConnection.h"

class TcpServer 
{
public:
	TcpServer(int threadCount = 4);
	virtual ~TcpServer();

	bool init(std::string ip, int port);
	void run();

private:
	bool bind(std::string ip, int port);
	bool listen();
	void onConnect();
	virtual void onRead(Packet& packet);
	virtual void onWrite(int bytes);
	virtual void onError(SOCKET socket);
	virtual void onClose(SOCKET socket);
	virtual void closeConnection();

private:
	std::shared_ptr<TcpSocket> m_sock;
	std::shared_ptr<EventLoop> m_loop;
	std::shared_ptr<TcpChannel> m_channel;
	std::shared_ptr<EventThreadPool> m_pool;
	std::map<SOCKET, std::unique_ptr<TcpConnection>> m_connections;
	std::list<SOCKET> m_disconnectingSock;
	std::mutex m_mutex;
};

#endif
