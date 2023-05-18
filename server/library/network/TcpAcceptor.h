#ifndef NETWORK_TCPACCEPTOR
#define NETWORK_TCPACCEPTOR

#include "../coroutine/Scheduler.h"
#include "./epoll/EpollWorker.h"
#include "./epoll/EpollChannel.h"
#include "./socket/TcpSocket.h"
#include "Callback.h"
#include <memory>

class TcpAcceptor
{
public:
    typedef std::shared_ptr<TcpAcceptor> ptr;
    TcpAcceptor(int threadCount);

	// addr = ip:port
	bool run(const std::string& addr);
	bool bind(const std::string& addr);
	bool listen();
    void exit();

	void onConnect();
	virtual void onError();

	void setErrorCallback(CALLBACK func);

	
private:
    std::shared_ptr<TcpSocket> m_sock;
    std::shared_ptr<Scheduler<EpollWorker>> m_listener;
	std::shared_ptr<Scheduler<EpollWorker>> m_epollWorkers;
    std::shared_ptr<EpollChannel> m_channel;
    std::atomic_int m_sessionId;
    
    CALLBACK m_errorCallback;
};

#endif