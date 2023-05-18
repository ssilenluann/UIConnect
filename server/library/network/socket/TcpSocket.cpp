#ifndef NETWORK_SOCKET_TCP_SOCKET_CPP
#define NETWORK_SOCKET_TCP_SOCKET_CPP

#include <unistd.h>
#include <fcntl.h>

#include "TcpSocket.h"
#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

TcpSocket::TcpSocket(const std::string& addr, int family, int type, int protocol, bool nonblock)
	: m_addr(Address::LookupAny(addr, family, type, protocol)), m_sock(Socket::CreateTCP(m_addr)),
	m_isNonblock(nonblock)
{
	if(m_isNonblock)	setNonblock();
}

TcpSocket::TcpSocket(int family, int type, int protocol, bool nonblock)
	: m_isNonblock(nonblock)
{
}

TcpSocket::TcpSocket(Socket::ptr sock, bool nonblock): m_sock(sock), m_isNonblock(nonblock)
{
}

TcpSocket::~TcpSocket()
{
	m_sock->close();
}

int TcpSocket::bind()
{
	return m_sock->bind(m_addr);
}

int TcpSocket::connect(const std::string& address)
{
	m_addr = Address::LookupAny(address);
	m_sock = Socket::CreateTCP(m_addr);
	if(m_isNonblock)	setNonblock();

	return m_sock->connect(m_addr);
}

bool TcpSocket::send(std::shared_ptr<Buffer>& buffer, int& sendSize)
{
	if(!m_sock->isValid())	return false;

	std::vector<iovec> iovs;
	buffer->getReadBuffers(iovs, buffer->getUnreadSize());

	sendSize = 0;
	int retp = m_sock->send(&iovs[0], iovs.size());
	if (retp > 0)
	{
		// move message from buffer
		sendSize += retp;
		buffer->hasRead(retp);
		return true;
	}

	if(retp == 0)
		return false;

	// interrupted or tcp send buffer is not enough, retry
	if(retp < 0 && (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR))
		return true;

	// other side closed or socket error
	LOG_FMT_ERROR(g_logger, "tcp send error, sock_fd = %d, errno = %d", m_sock, errno);
	sendSize = -1;
	return false;
}

bool TcpSocket::recv(std::shared_ptr<Buffer>& buffer, int& recvSize)
{
	if (!m_sock->isValid()) return false;

	int retp = 0;
	recvSize = 0;

	// dont't know how much to read, so create a bigger extrabuf,
	// if some data read into extrabuf, append it into buffer later
	std::shared_ptr<char> extrabuf(new char[65536], [](char* ptr){ delete[] ptr;});

	// keep receiving until all data retrieved if socket is nonblock
	while(true)
	{
		memset(extrabuf.get(), 0, 65536);

		std::vector<iovec> iovs;
		buffer->getReadBuffers(iovs, buffer->getUnreadSize());
		
		iovec extraIov;
		extraIov.iov_base = extrabuf.get();
		extraIov.iov_len = 65536;
		iovs.push_back(extraIov);

		// received
		retp = m_sock->recv(&iovs[0], iovs.size());
		if(retp > 0)
		{
			int extraReadSize = retp - buffer->getFreeSize();
			if(extraReadSize > 0)
			{
				buffer->hasWritten(buffer->getFreeSize());
				
				// add message from exter buffer into buffer
				buffer->write(extrabuf.get(), extraReadSize);
				buffer->hasWritten(extraReadSize);
			}
			else
			{
				buffer->hasWritten(retp);
			}
			recvSize += retp;
			
			if(m_isNonblock) continue;
			else break;	// block socket, return
		}

		// other side closed
		if(retp == 0)
		{
			recvSize = 0;
			return false;
		}
					
		// retp < 0, check errno
		if(retp < 0 && errno == EINTR)	// interupted, try again
			continue;
		
		if(retp < 0 && errno == EWOULDBLOCK)  // all data retrieved
		{
			return true;
		}
		
		if(retp < 0)	// socket error
		{
			recvSize = -1;
			LOG_FMT_ERROR(g_logger, "tcp recv error, sock_fd = %d, errno = %d", m_sock, errno);
			return false;
		}
	}

	return true;
}

std::string TcpSocket::getLastError()
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

	return m_sock->getOption(SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0
		? strerror(errno) : strerror(optval);
}

void TcpSocket::setNonblock()
{
	int oldFlag = fcntl(m_sock->getSocket(), F_GETFL, 0);
	int newFlag = oldFlag | O_NONBLOCK;

	fcntl(m_sock->getSocket(), F_SETFL, newFlag);
	m_isNonblock = true;
}

void TcpSocket::initSock()
{
	m_sock->initSock();
	if(m_isNonblock)	setNonblock();
}

bool TcpSocket::init(int sock)
{
    m_sock->init(m_sock->getSocket());
	if(m_isNonblock)	setNonblock();
}

void TcpSocket::listen(int backlog)
{
	m_sock->listen(backlog);
}

TcpSocket::ptr TcpSocket::accept()
{
	return std::make_shared<TcpSocket>(m_sock->accept());
}

void TcpSocket::close()
{
	m_sock->close();
}


#endif
