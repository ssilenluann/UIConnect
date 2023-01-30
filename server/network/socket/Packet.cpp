#ifndef NETWORK_PACKET_CPP
#define NETWORK_PACKET_CPP

#include "Packet.h"

Packet::Packet(WORD command) : head(0xFEFF), length(0), cmd(command), dataLoadPos(0), dataLoadSize(0), checkSum(0) {}

Packet::Packet(WORD command, const char* data, int size)
{
	head = 0xFEFF;
	length = size + sizeof(cmd) + sizeof(checkSum);
	cmd = command;
	dataLoadPos = sizeof(head) + sizeof(length) + sizeof(cmd);
	dataLoadSize = size;
	checkSum = 0;
	for (int i = 0; i < size; i++)
	{
		checkSum += BYTE(data[i]) & 0xFF;
	}

	int packSize = length + sizeof(length) + sizeof(head);
	packData.resize(packSize);
	char* pData = (char*)packData.c_str();

	*(WORD*)pData = head;
	pData += sizeof(WORD);

	*(DWORD*)pData = length;
	pData += sizeof(DWORD);

	*(WORD*)pData = cmd;
	pData += sizeof(WORD);

	memcpy(pData, data, size);
	pData += size;

	*(WORD*)pData = checkSum;
	pData += sizeof(WORD);
}

Packet::Packet(const Packet& pack) : head(0xFEFF), length(pack.length), cmd(pack.cmd),
	dataLoadPos(pack.dataLoadPos), dataLoadSize(pack.dataLoadSize), checkSum(pack.checkSum), packData(pack.packData) {}

Packet& Packet::operator=(const Packet& rhs)
{
	if (this != &rhs)
	{
		head = rhs.head;
		length = rhs.length;
		cmd = rhs.cmd;
		dataLoadPos = rhs.dataLoadPos;
		dataLoadSize = rhs.dataLoadSize;
		checkSum = rhs.checkSum;
		packData = rhs.packData;
	}

	return *this;

}

#endif