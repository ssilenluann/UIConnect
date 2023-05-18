#ifndef NETWORK_TCPSERVER_H
#define NETWORK_TCPSERVER_H

#include <memory>
#include <vector>
#include <atomic>
#include <list>
#include "./socket/TcpSocket.h"
#include "./epoll/EpollWorker.h"
#include "./TcpSession.h"
#include "Callback.h"

class TcpServer 
{
public:
	typedef std::shared_ptr<TcpServer> ptr;
	
	TcpServer(int threadCount = 4);
	virtual ~TcpServer();

	bool init(std::string ip, int port);
	void run();

private:
	bool bind(std::string ip, int port);
	bool listen();
	void onConnect();
	virtual void onError();
	virtual void exit();

	void setStartCallback(CALLBACK func);
	void setErrorCallback(CALLBACK func);
	void setQuitCallback(CALLBACK func);

private:
	std::shared_ptr<TcpSocket> m_sock;
	std::shared_ptr<EpollChannel> m_channel;
	std::shared_ptr<Scheduler<EpollWorker>> m_epollWorkers;
	std::shared_ptr<EpollWorker> m_acceptor;
	std::mutex m_mutex;
	std::atomic_int m_sessionId;
	bool m_isStoped;
	

	CALLBACK m_startCallback;
	CALLBACK m_errorCallback;
	CALLBACK m_quitCallback;
	
};

#endif
