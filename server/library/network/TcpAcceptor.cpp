#include "TcpAcceptor.h"
#include "TcpConnection.h"
#include "TcpSession.h"
#include "Socket.h"
static Logger::ptr g_logger = LOG_NAME("system");

TcpAcceptor::TcpAcceptor(int threadCount): m_listener(new Scheduler<EpollWorker>(1))
    , m_epollWorkers(new Scheduler<EpollWorker>(threadCount)), m_sessionId(0)
{
}

bool TcpAcceptor::run(const std::string &addr)
{
	if(!bind(addr) || !listen())
	{
		LOG_FATAL(g_logger) << "server init failed";
		return false;
	}

	auto epollWorker = m_listener->getNextWorker();
	m_channel.reset(new EpollChannel(epollWorker, m_sock->fd()));
	m_channel->setReadCallback(std::bind(&TcpAcceptor::onConnect, this));
	m_channel->setErrorCallback(std::bind(&TcpAcceptor::onError, this));
	m_channel->enableReading();

    m_epollWorkers->start();
    m_listener->start();

	m_listener->waitForQuit();
	m_epollWorkers->waitForQuit();
}

bool TcpAcceptor::bind(const std::string &addr)
{
	LOG_DEBUG(g_logger) << "create tcp socket, addr: " << addr;
	m_sock.reset(new TcpSocket(addr));
	int retp =  m_sock->bind();
	if(retp < 0)
	{
		LOG_FMT_FATAL(g_logger, "server bind port error, socket fd = %d, errno = %d", m_sock->fd(), errno);
	}
	return retp >= 0;
}

bool TcpAcceptor::listen()
{
	m_sock->listen();
}

void TcpAcceptor::exit()
{
	// 1. disable listen channel
	m_channel->disable();
	// 2. quit listen loop
	// m_workers->stop();	
	m_listener->stop();
	// 3. close thread pool
	m_epollWorkers->stop();
	// 4.close listen sock
	m_sock->close();
}

void TcpAcceptor::onConnect()
{
	TcpSocket::ptr sock = m_sock->accept();
	sock->setNonblock();
	if(!sock->isValid())
	{
		LOG_INFO(g_logger) << "accept failed";
		return;
	}
	
	std::shared_ptr<EpollWorker> worker = m_epollWorkers->getNextWorker();
	m_sessionId++;
	std::unique_ptr<TcpConnection> connection(new TcpConnection(sock, worker));
	worker->addSession(std::make_shared<TcpSession>(m_sessionId, std::move(connection), worker));

}

void TcpAcceptor::onError()
{
	std::cout << m_sock->fd() << " error, msg: " << m_sock->getLastError() << std::endl;
    if(m_errorCallback)
        m_errorCallback();
}

void TcpAcceptor::setErrorCallback(CALLBACK func)
{
	m_errorCallback = func;
}
