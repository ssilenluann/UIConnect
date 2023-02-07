#ifndef NETWORK_SOCKET_TCP_SOCKET_H
#define NETWORK_SOCKET_TCP_SOCKET_H

#include "Socket.hpp"
#include "Packet.h"
#include "Buffer.h"
#include "SockAddr.hpp"

#include <memory>
class TcpSocket
{
public:
	TcpSocket(bool nonblock = true);
	TcpSocket(SOCKET sock);
	TcpSocket(const TcpSocket& sock);

	TcpSocket& operator=(const TcpSocket& rhs);
	~TcpSocket();
public:
	SOCKET fd();

	void reuse();

	int bind(const SockAddr& addr);
	int bind(std::string ip, unsigned short port);

	int listen(int backlog = 5);

	SOCKET accept(SockAddr& addr);

	int connect(const SockAddr& addr);
	int connect(std::string ip, unsigned short port);

	bool send(std::shared_ptr<Buffer>& buffer, int& sendSize);

	bool recv(std::shared_ptr<Buffer>& buffer, int& recvSize);

	std::shared_ptr<Packet> getPack();

	inline SOCKET fd() const;
	inline std::shared_ptr<Socket> sock();

	inline bool isValid();
	
	void close();

	std::string getLastError();

private:
	std::shared_ptr<Socket> m_sock;
	int m_errno;
	bool m_isNonBlock;

};
#endif
