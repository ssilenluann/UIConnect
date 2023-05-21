#ifndef NETWORK_SOCKET_BUFFER_CPP
#define NETWORK_SOCKET_BUFFER_CPP
#include "Buffer.h"
#include "../../log/Logger.h"

#include <algorithm>
static Logger::ptr g_logger = LOG_NAME("system");

bool Buffer::setMsg(const char* msg, int size)
{
	write(msg, size);
	hasWritten(size);
	return true;
}

std::shared_ptr<Packet> Buffer::getPack()
{
	std::shared_ptr<Packet> pack = std::make_shared<Packet>(DWORD(PackCommand::INVALID));
    getPack(pack.get());
	return pack;
}

void Buffer::getPack(Packet& pack)
{
    return getPack(&pack);
}

void Buffer::getPack(Packet* pack)
{
	int index = 0;
	// get header
	size_t tmpPos = m_readPos, packStartIndex = 0;
	
	for(;;)
	{
		uint16_t header = 0;
		read(&header, sizeof(header), tmpPos);
		tmpPos += sizeof(header);
		
		if(header == 0xFEFF || tmpPos > getSize())	break;
	}

	packStartIndex = tmpPos - sizeof(pack->head);
	pack->head = 0xFEFF;
	if (getSize() - tmpPos < sizeof(pack->length))
		return;

	read(&pack->length, sizeof(pack->length), tmpPos);
	tmpPos += sizeof(pack->length);

	if(getSize() - tmpPos < pack->length)
		return;

	read(&pack->cmd, sizeof(pack->cmd), tmpPos);
	tmpPos += sizeof(pack->cmd);

	pack->dataLoadPos = sizeof(pack->head) + sizeof(pack->length) + sizeof(pack->cmd);
	pack->dataLoadSize = pack->length - (sizeof(pack->cmd) + sizeof(pack->checkSum));
	
	read(&pack->checkSum, sizeof(pack->checkSum), packStartIndex + pack->length - sizeof(pack->checkSum));
	
	pack->packData.resize(pack->length);
	read(const_cast<char*>(pack->packData.c_str()), pack->length, packStartIndex);

	uint16_t sum = 0;
	for (int i = pack->dataLoadPos; i < pack->dataLoadPos + pack->dataLoadSize; i++)
	{
		sum += (uint8_t(pack->packData[i]) & 0xFF);
	}

	if (sum != pack->checkSum)
	{
		pack->cmd = uint64_t(PackCommand::INVALID);
	}

	tmpPos = packStartIndex + pack->length;
	hasRead(tmpPos - m_readPos);
}


int Buffer::find(const std::string & str, std::string& prefix, int readLength)
{
	readLength = std::min((size_t)readLength, getUnreadSize());
	if(readLength == 0)	return std::string::npos;
	
	std::string buf;
	buf.resize(readLength);

	read(static_cast<void*>(&buf[0]), readLength);

	int pos = buf.find(str);
	if(pos == std::string::npos)	return pos;

	prefix = buf.substr(0, pos);
	return pos;
}

#endif

