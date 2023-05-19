#ifndef NETWORK_SOCKET_SOCKET_HPP
#define NETWORK_SOCKET_SOCKET_HPP

#ifndef _WIN32
	typedef int SOCKET;
	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET (0)
	#endif
	#ifndef SOCKET_ERROR
		#define SOCKET_ERROR (int(-1))
	#endif
#endif

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef _WIN32
class SocketIniter;
#endif 

class Sock 
{
public:
	// RAII
	Sock(bool nonblock = true): m_sock(INVALID_SOCKET)
	{
		// SOCK_CLOEXEC, close for exec, for example, if a child process opened by exec,
		// this socket is unavailable for it
		int type = SOCK_STREAM | SOCK_CLOEXEC | (nonblock ? SOCK_NONBLOCK : 0);
		m_sock = socket(AF_INET, type, 0);
		if(m_sock == SOCKET_ERROR)
		{
			// TODO: LOG
			m_sock = INVALID_SOCKET;
			return;
		}
	}

	Sock(SOCKET sock) : m_sock(sock) {}
	
	Sock(const Sock& sock) : m_sock(sock.m_sock) {}

	Sock& operator=(const Sock& rhs)
	{
		if (this != &rhs)
		{
			m_sock = m_sock;
		}
		return *this;
	}

	operator const SOCKET () const
	{
		return m_sock;
	}

	// RAII
	virtual ~Sock()
	{
		close();
	}
	
	inline SOCKET fd()
	{
		return m_sock;
	}

public:
	void close()
	{
		if (m_sock != INVALID_SOCKET)
		{
			::close(m_sock);
			m_sock = INVALID_SOCKET;
		}
	}

protected:
#ifdef _WIN32
	static SocketIniter m_sockIniter;
#endif
	SOCKET m_sock;
};

// init socket environment in Windowns
#ifdef _WIN32
class SocketIniter
{
public:
	SocketIniter()
	{

		WSADATA data;
		WSAStartup(MAKEWORD(2,2), &data);
	}

	~SocketIniter()
	{

		WSACleanup();
	}
};
SocketIniter Sock::m_sockIniter;
#endif
#endif
