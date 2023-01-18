#ifndef NETWORK_SOCKET_PACKET_H
#define NETWORK_SOCKET_PACKET_H

#include <string>
#include <memory>
#include <cstring>

#ifndef _WIN32
	typedef unsigned short WORD;
	typedef unsigned long DWORD;
	typedef unsigned char BYTE;
#endif

enum class PackCommand
{
	INVALID = 0, 
	VEDIO_INFO = 1,
};

//packet foramt: head(FF FE) length cmd dataload checksum 
//length =  nCmd(2) + dataLoad.size() + checkSum(2)
class Packet
{

public:
	Packet(WORD command = WORD(PackCommand::INVALID));

	Packet(WORD command, const char* data, int size);

	Packet(const Packet& pack);
	Packet& operator=(const Packet& rhs);

	inline std::string dataLoad() const;

	inline DWORD packSize() const;
	inline bool isValid();

	inline void clear()
	{
		cmd = DWORD(PackCommand::INVALID);
		packData.clear();
	}

public:
	WORD head;				// fix head data: FF FE
	DWORD length;			// pack size
	WORD cmd;				// command
	DWORD dataLoadPos;	    // valid message head
	DWORD dataLoadSize;	    // valid message size
	WORD checkSum;			// check sum
	std::string packData;	// whole pack data
};

#endif
