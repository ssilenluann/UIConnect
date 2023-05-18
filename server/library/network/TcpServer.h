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
#include "TcpAcceptor.h"

class TcpServer 
{
public:
	typedef std::shared_ptr<TcpServer> ptr;
	
	TcpServer(int threadCount = 4);
	virtual ~TcpServer();

	void run(const std::string& addr);
	virtual void exit();

private:

	void setStartCallback(CALLBACK func);
	void setErrorCallback(CALLBACK func);
	void setQuitCallback(CALLBACK func);

private:
	TcpAcceptor::ptr m_acceptor;
	std::mutex m_mutex;
	bool m_isStoped;

	CALLBACK m_startCallback;
	CALLBACK m_quitCallback;
	
};

#endif
