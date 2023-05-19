#ifndef NETWORK_SOCKET_BUFFER_CPP
#define NETWORK_SOCKET_BUFFER_CPP
#include "Buffer.h"
#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

Buffer::Buffer() :m_pos(0)
{
	m_pos = 0;
	memset(m_buffer, 0, BUFFER_SIZE);
}

Buffer::Buffer(const Buffer& buf)
{
	m_pos = buf.m_pos;
	memcpy(m_buffer, buf.m_buffer, BUFFER_SIZE);

}

Buffer& Buffer::operator=(const Buffer& rhs)
{
	if (&rhs != this)
	{
		m_pos = rhs.m_pos;
		memcpy(m_buffer, rhs.m_buffer, BUFFER_SIZE);
	}
	return *this;
}

void Buffer::operator-=(int rhs) { m_pos -= rhs; }

bool Buffer::setMsg(const char* msg, int size)
{
	if (freeSize() < size)
		return false;

	memcpy(m_buffer + m_pos, msg, size);
	m_pos += size;

	return true;
}

void Buffer::remove(int size)
{
	if (size > m_pos)	return;
	memcpy(m_buffer, m_buffer + size, m_pos - size);
	m_pos -= size;
	memset(m_buffer + m_pos, 0, size);

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

int Buffer::find(const char * str, int len)
{
	for(int i = 0; i < m_pos; i++)
	{
		if(strncmp(str, &m_buffer[i], len) == 0)
			return i;
	}

	return -1;
}

void Buffer::getPack(Packet* pack)
{
	int index = 0;
	// get header
	while (index < m_pos)
	{
		if (*(WORD*)(m_buffer + index) == 0xFEFF)
			break;
		index++;
	}

	if (index == m_pos || m_pos < int(index + sizeof(pack->cmd) + sizeof(pack->checkSum)))
		return;

	pack->head = 0xFEFF;
	index += sizeof(pack->head);

	pack->length = *(DWORD*)(m_buffer + index);
	index += sizeof(pack->length);

	if (m_pos < pack->length + (sizeof(pack->head) + sizeof(pack->length)))
		return;

	pack->cmd = *(WORD*)(m_buffer + index);
	index += sizeof(pack->cmd);

	pack->dataLoadPos = sizeof(pack->head) + sizeof(pack->length) + sizeof(pack->cmd);
	pack->dataLoadSize = pack->length - (sizeof(pack->cmd) + sizeof(pack->checkSum));
	index += pack->dataLoadSize;

	pack->checkSum = *(WORD*)(m_buffer + index);
	index += sizeof(pack->checkSum);

	WORD sum = 0;
	for (int i = pack->dataLoadPos; i < pack->dataLoadPos + pack->dataLoadSize; i++)
	{
		sum += (BYTE(m_buffer[i]) & 0xFF);
	}

	if (sum != pack->checkSum)
	{
		pack->cmd = DWORD(PackCommand::INVALID);
	}

	pack->packData.resize(index);
	memcpy((char*)pack->packData.c_str(), m_buffer, index);

	remove(index);
}

#endif