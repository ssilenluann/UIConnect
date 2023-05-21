#ifndef NETWORK_SOCKET_BUFFER_H
#define NETWORK_SOCKET_BUFFER_H

#include "Packet.h"
#include "../../ByteArray.h"

class Buffer: public ByteArray
{

public:
	Buffer() = default;

	inline bool setMsg(const Packet& pack) { return setMsg(pack.packData.c_str(), pack.packSize());}
	inline bool setMsg(const std::shared_ptr<Packet>& pack) { return setMsg(pack->packData.c_str(), pack->packSize());}
	bool setMsg(const char* msg, int size);
	
	std::shared_ptr<Packet> getPack();
	void getPack(Packet& pack);

	int find(const std::string& str, std::string& prefix, int readLength = 128);

private:
	void getPack(Packet* pack);

};

#endif