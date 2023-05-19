#ifndef NETWORK_SOCKET_TCP_SOCKET_CPP
#define NETWORK_SOCKET_TCP_SOCKET_CPP

#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "TcpSocket.h"
#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

TcpSocket::TcpSocket(bool nonblock) 
	:m_sock(new Sock(nonblock)), m_isNonBlock(nonblock)
{
	reuse();
}

TcpSocket::TcpSocket(SOCKET sock) 
	:m_sock(new Sock(sock)) {}

TcpSocket::TcpSocket(const TcpSocket& sock) 
	: m_sock(sock.m_sock){}

TcpSocket& TcpSocket::operator=(const TcpSocket& rhs)
{
	if (&rhs != this)
	{
		m_sock = rhs.m_sock;
	}
	return *this;
}

TcpSocket::~TcpSocket()
{
	close();
}

SOCKET TcpSocket::fd()
{
	return *m_sock;
}

// reuse socket to bind in TIME_WAIT period
void TcpSocket::reuse()
{
	int on = 1;
	setsockopt(*m_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	setsockopt(*m_sock, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on));
}

int TcpSocket::bind(const SockAddr& addr)
{
	if (!isValid())
		return SOCKET_ERROR;

	int size = addr.size();
	if(::bind(*m_sock, addr, size) == -1)
	{
		LOG_FMT_FATAL(g_logger, "tcp bind error, sock_fd = %d, errno = %d, %s", m_sock->fd(), errno, strerror(errno));
		close();
		return SOCKET_ERROR;
	}

	LOG_DEBUG(g_logger) << "tcp bind seccessufully";
	return 0;
}

int TcpSocket::bind(std::string ip, unsigned short port)
{
	return bind(SockAddr(ip, port));
}

int TcpSocket::listen(int backlog)
{
	if (!isValid())
		return SOCKET_ERROR;

	if(::listen(*m_sock, backlog) == SOCKET_ERROR)
	{
		LOG_FMT_FATAL(g_logger, "tcp listen error, sock_fd = %d, errno = %d", m_sock->fd(), errno, strerror(errno));
		close();
		return SOCKET_ERROR;
	}
	return 0;
}

SOCKET TcpSocket::accept(SockAddr& addr)
{
	if (!isValid())
		return SOCKET_ERROR;

	socklen_t size = addr.size();
	int fd = ::accept4(*m_sock, addr, &size, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if(fd < 0)
	{
		LOG_FMT_ERROR(g_logger, "tcp accept error, sock_fd = %d, errno = %d", m_sock->fd(), errno, strerror(errno));
		close();
		return SOCKET_ERROR;
	}
	
	return fd;
}

int TcpSocket::connect(const SockAddr& addr)
{
	if (!isValid())
		return SOCKET_ERROR;

	socklen_t size = addr.size();
	int retp = ::connect(*m_sock, addr, size);

	if(retp != SOCKET_ERROR) return retp;	// connected

	// not connected: check if connecting or failed
	if(errno == EINPROGRESS || errno == EINTR || errno == EISCONN)
		return 0;

	fd_set writefd;
	FD_SET(*m_sock, &writefd);

	timeval time;
	memset(&time, 0, sizeof(timeval)); 
	retp = select(*m_sock + 1, nullptr, &writefd, nullptr, &time);
	if(retp < 0 || !FD_ISSET(*m_sock, &writefd))
		return SOCKET_ERROR;

	return 0;
}

int TcpSocket::connect(std::string ip, unsigned short port)
{
	return connect(SockAddr(ip, port));
}

/**
 * @brief buffer data out
 * @return
 * 		1.socket_error: sendSize = -1, return false; 
 * 		2.send successfully: sendSize > 0, return true; 
 * 		3.other side closed: sendSize = 0, return false;
 * 		4.signal: EWOULDBLOCK, EAGAIN, EINTR : sendSize = 0, return true;
 * */
bool TcpSocket::send(std::shared_ptr<Buffer>& buffer, int& sendSize)
{
	if(!isValid())	return false;

	sendSize = 0;
	int retp = ::send(*m_sock, buffer->start(), buffer->pos(), 0);
	if (retp > 0)
	{
		// move message from buffer
		sendSize += retp;
		buffer->remove(retp);
		return true;
	}

	if(retp == 0)
		return false;

	// interrupted or tcp send buffer is not enough, retry
	if(retp < 0 && (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR))
		return true;

	// other side closed or socket error
	LOG_FMT_ERROR(g_logger, "tcp send error, sock_fd = %d, errno = %d", m_sock->fd(), errno);
	sendSize = -1;
	return false;
}

/**
 * @brief try to retrieve all data from tcp buffer,
 * 		  if given buffer is not large enough, retry next time
 * @return
 * 		1.socket_error: receSize = -1, return false
 * 		2.other side closed: recvSize = 0, return false
 * 		3.recv successfully: return true
 * 						   if buffer is fulled at the first time, recvSize = 0
 * */ 
bool TcpSocket::recv(std::shared_ptr<Buffer>& buffer, int& recvSize)
{
	if (!isValid()) return false;

	int retp = 0;
	recvSize = 0;

	// keep receiving until all data retrieved or given buffer is fulled if socket is nonblock
	while(true)
	{
		if(buffer->fulled())
		{
			return true;
		}

		// received
		retp = ::recv(*m_sock, buffer->end(), buffer->freeSize(), 0);
		if(retp > 0)
		{
			buffer->addPos(retp);
			recvSize += retp;
			
			if(m_isNonBlock) continue;
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
			LOG_FMT_ERROR(g_logger, "tcp recv error, sock_fd = %d, errno = %d", m_sock->fd(), errno);
			return false;
		}
	}

	return true;
}

std::shared_ptr<Sock> TcpSocket::sock() { return m_sock; }

bool TcpSocket::isValid() { return m_sock != nullptr && *m_sock > SOCKET(0); }

void TcpSocket::close()
{
	if (isValid())
		m_sock->close();

	LOG_FMT_INFO(g_logger, "tcp closed, sock_fd = %d", m_sock->fd());

}

std::string TcpSocket::getLastError()
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

	return getsockopt(*m_sock, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0
		? strerror(errno) : strerror(optval);
}
#endif
