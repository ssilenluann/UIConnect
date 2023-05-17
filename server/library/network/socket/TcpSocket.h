#ifndef NETWORK_SOCKET_TCP_SOCKET_H
#define NETWORK_SOCKET_TCP_SOCKET_H

#include "Socket.h"
#include "Packet.h"
#include "Buffer.h"
#include "Address.h"
#include "../../Noncopiable.h"

#include <memory>
class TcpSocket: public Socket, public Noncopyable
{
public:
	typedef std::shared_ptr<TcpSocket> ptr;
	
	TcpSocket(int family, int type, int protocol = 0, bool nonblock = true);
	~TcpSocket();

public:

	int bind(std::string ip, unsigned short port);
	int connect(std::string ip, unsigned short port);
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

	virtual void initSock();
    virtual bool init(int sock);

private:
	bool m_isNonblock;

};
#endif
