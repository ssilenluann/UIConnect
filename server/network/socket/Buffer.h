#ifndef NETWORK_SOCKET_BUFFER_H
#define NETWORK_SOCKET_BUFFER_H
#include "Packet.h"

#define BUFFER_SIZE (1024*4)

class Buffer
{
public:
	Buffer();

	Buffer(const Buffer& buf);
	Buffer& operator=(const Buffer& rhs);

	inline operator char* () { return m_buffer; }
	inline operator const char* () const{ return m_buffer; }
	void operator-=(int rhs);

	inline int freeSize() { return BUFFER_SIZE - m_pos; }
	inline int pos() { return m_pos; }
	inline char* end() { return m_buffer + m_pos; }
	inline char* start() { return m_buffer; }
    inline bool fulled() { return m_pos == BUFFER_SIZE;}

	inline bool setMsg(const Packet& pack) { return setMsg(pack.packData.c_str(), pack.packSize());}
	inline bool setMsg(const std::shared_ptr<Packet>& pack) { return setMsg(pack->packData.c_str(), pack->packSize());}
	bool setMsg(const char* msg, int size);
	void remove(int size);
	inline void setPos(int pos) { m_pos = pos;}
	inline void addPos(int pos) { m_pos += pos;}
	std::shared_ptr<Packet> getPack();
	void getPack(Packet& pack);
	inline void clear()
	{
		memset(m_buffer, 0, BUFFER_SIZE);
		m_pos = 0;
	}
	
private:
	void getPack(Packet* pack);

private:
	char m_buffer[BUFFER_SIZE];
	int m_pos;
	
};

#endif