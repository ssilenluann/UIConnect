#ifndef NETWORK_SOCKET_TCP_SOCKET_H
#define NETWORK_SOCKET_TCP_SOCKET_H

#include "Socket.h"
#include "Packet.h"
#include "Buffer.h"
#include "Address.h"
#include "../../Noncopiable.h"

#include <memory>
class TcpSocket
{
public:
	typedef std::shared_ptr<TcpSocket> ptr;
	
	TcpSocket(const std::string& addr, int family = AF_INET, int type = 0, int protocol = 0, bool nonblock = true);
	TcpSocket(int family = AF_INET, int type = 0, int protocol = 0, bool nonblock = true);
	TcpSocket(Socket::ptr sock, bool nonblock = true);

	~TcpSocket();

public:

	int bind();
	// adderess: ip:port
	int connect(const std::string& address);
	/**
	 * @brief buffer data out
	 * @return
	 * 		1.socket_error: sendSize = -1, return false; 
	 * 		2.send successfully: sendSize > 0, return true; 
	 * 		3.other side closed: sendSize = 0, return false;
	 * 		4.signal: EWOULDBLOCK, EAGAIN, EINTR : sendSize = 0, return true;
	 * */
	bool send(std::shared_ptr<Buffer>& buffer, int& sendSize);

	/**
	 * @brief try to retrieve all data from tcp buffer,
	 * 		  if given buffer is not large enough, retry next time
	 * @return
	 * 		1.socket_error: receSize = -1, return false
	 * 		2.other side closed: recvSize = 0, return false
	 * 		3.recv successfully: return true
	 * 						   if buffer is fulled at the first time, recvSize = 0
	 * */ 
	bool recv(std::shared_ptr<Buffer>& buffer, int& recvSize);
	
	std::string getLastError();
	inline bool isNonblock() const { return m_isNonblock;}
	void setNonblock();

	bool isValid() { return m_sock->isValid();}
	bool fd() { return m_sock->getSocket();}

	virtual void initSock();
    virtual bool init(int sock);

	void listen(int backlog = 4096);
	TcpSocket::ptr accept();
	void close();

private:
	Address::ptr m_addr;
	Socket::ptr m_sock;
	bool m_isNonblock;

};
#endif
