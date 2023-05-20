#ifndef NETWORK_SOCKET_SOCK_ADDR_HPP
#define NETWORK_SOCKET_SOCK_ADDR_HPP

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include "windows.h"
#include "Winsock2.h"
#endif 

#include <string>
#include <cstring>
#include <arpa/inet.h>

class SockAddr
{
public:
	SockAddr() : m_port(-1) { memset(&m_addr, 0, sizeof(sockaddr_in)); }

	SockAddr(std::string ip, unsigned short port) : m_ip(ip), m_port(port)
	{
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	}

	SockAddr(const SockAddr& addr)
	{
		memcpy(&m_addr, &addr.m_addr, sizeof(sockaddr_in));
		m_ip = addr.m_ip;
		m_port = addr.m_port;
	}

	SockAddr& operator=(const SockAddr& rhs)
	{
		if (&rhs != this)
		{
			memcpy(&m_addr, &rhs.m_addr, sizeof(sockaddr_in));
			m_ip = rhs.m_ip;
			m_port = rhs.m_port;
		}

		return *this;
	}

	socklen_t size() const 
	{
		return sizeof(sockaddr_in);
	}

	operator const sockaddr* () const
	{
		return (sockaddr*)&m_addr;
	}

	operator sockaddr* ()
	{
		return (sockaddr*)&m_addr;
	}

	void update()
	{
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);
	}

	const std::string& ip() const
	{
		return m_ip;
	}

	const unsigned short port() const
	{
		return m_port;
	}

private:
	sockaddr_in m_addr;
	std::string m_ip;
	unsigned short m_port;
};

#endif