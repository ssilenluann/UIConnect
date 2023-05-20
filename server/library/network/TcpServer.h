#ifndef NETWORK_TCPSERVER_H
#define NETWORK_TCPSERVER_H

#include <memory>
#include <vector>
#include <atomic>
#include <list>
#include "./socket/TcpSocket.h"
#include "./event/EventLoop.h"
#include "./event/EventThreadPool.h"
#include "./TcpSession.h"
#include "Callback.h"

class TcpServer 
{
public:
	TcpServer(int threadCount = 4);
	virtual ~TcpServer();

	virtual bool init(std::string ip, int port);
	void run();

private:
	bool bind(std::string ip, int port);
	bool listen();
	virtual void onConnect();
	virtual void onError();
	virtual void exit();

	void setStartCallback(CALLBACK func);
	void setErrorCallback(CALLBACK func);
	void setQuitCallback(CALLBACK func);

protected:
	std::shared_ptr<EventLoop> m_loop;
	std::shared_ptr<TcpSocket> m_sock;
	std::shared_ptr<TcpChannel> m_channel;
	std::shared_ptr<EventThreadPool> m_pool;
	std::mutex m_mutex;
	std::atomic_int m_sessionId;

	CALLBACK m_startCallback;
	CALLBACK m_errorCallback;
	CALLBACK m_quitCallback;
	
};

#endif
