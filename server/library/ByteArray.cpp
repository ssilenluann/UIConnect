#ifndef BYTEARRAY_CPP
#define BYTEARRAY_CPP

#include <fstream>
#include <sstream>
#include <string.h>
#include <iomanip>
#include <math.h>

#include "ByteArray.h"
#include "Endian.h"
#include "log/Logger.h"

static Logger::ptr g_logger = LOG_NAME("system");

ByteArray::Node::Node(size_t s)
    :ptr(new char[s])
    ,next(nullptr)
    ,size(s) 
{
}

ByteArray::Node::Node()
    :ptr(nullptr)
    ,next(nullptr)
    ,size(0) 
{
}

ByteArray::Node::~Node()
{
    if(ptr) {
        delete[] ptr;
    }
}

ByteArray::ByteArray(size_t base_size)
    :m_baseSize(base_size)
    ,m_writePos(0)
    ,m_readPos(0)
    ,m_capacity(base_size)
    ,m_endian(BIG_ENDIAN)
    ,m_root(new Node(base_size))
    ,m_writeBlock(m_root) 
    ,m_readBlock(m_root) 
{
}

ByteArray::~ByteArray() 
{
    Node* tmp = m_root;
    while(tmp) {
        m_writeBlock = tmp;
        tmp = tmp->next;
        delete m_writeBlock;
    }
}

uint32_t ByteArray::EncodeZigzag32(const int32_t& v) 
{
    if(v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

uint64_t ByteArray::EncodeZigzag64(const int64_t& v) 
{
    if(v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

int32_t ByteArray::DecodeZigzag32(const uint32_t& v) 
{
    return (v >> 1) ^ -(v & 1);
}

int64_t ByteArray::DecodeZigzag64(const uint64_t& v) 
{
    return (v >> 1) ^ -(v & 1);
}

bool ByteArray::isLittleEndian() const
{
    return m_endian == LITTLE_ENDIAN;
}

void ByteArray::setIsLittleEndian(bool val)
{
    if(val) {
        m_endian = LITTLE_ENDIAN;
    } else {
        m_endian = BIG_ENDIAN;
    }
}

void ByteArray::writeFint8  (int8_t value) 
{
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8 (uint8_t value) 
{
    write(&value, sizeof(value));
}

void ByteArray::writeFint16 (int16_t value) 
{
    if(m_endian != BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) 
{
    if(m_endian != BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32 (int32_t value) 
{
    if(m_endian != BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) 
{
    if(m_endian != BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64 (int64_t value) 
{
    if(m_endian != BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) 
{
    if(m_endian != BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeInt32  (int32_t value) 
{
    writeUint32(EncodeZigzag32(value));
}

void ByteArray::writeUint32 (uint32_t value) 
{
    uint8_t tmp[5];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64  (int64_t value) 
{
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64 (uint64_t value) 
{
    uint8_t tmp[10];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeFloat  (float value) 
{
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble (double value) 
{
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}

void ByteArray::writeStringF16(const std::string& value) 
{
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string& value) 
{
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string& value) 
{
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string& value) 
{
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string& value) 
{
    write(value.c_str(), value.size());
}

int8_t   ByteArray::readFint8(size_t startPos) 
{
    int8_t v;
    read(&v, sizeof(v), startPos);
    return v;
}

uint8_t  ByteArray::readFuint8(size_t startPos) 
{
    uint8_t v;
    read(&v, sizeof(v), startPos);
    return v;
}

#define XX(type, startpos) \
    type v; \
    read(&v, sizeof(v), startpos); \
    if(m_endian == BYTE_ORDER) { \
        return v; \
    } else { \
        return byteswap(v); \
    }

int16_t  ByteArray::readFint16(size_t startPos) 
{
    XX(int16_t, startPos);
}
uint16_t ByteArray::readFuint16(size_t startPos) 
{
    XX(uint16_t, startPos);
}

int32_t  ByteArray::readFint32(size_t startPos) 
{
    XX(int32_t, startPos);
}

uint32_t ByteArray::readFuint32(size_t startPos) 
{
    XX(uint32_t, startPos);
}

int64_t  ByteArray::readFint64(size_t startPos) 
{
    XX(int64_t, startPos);
}

uint64_t ByteArray::readFuint64(size_t startPos) 
{
    XX(uint64_t, startPos);
}

#undef XX

int32_t  ByteArray::readInt32(size_t startPos) 
{
    return DecodeZigzag32(readUint32(startPos));
}

uint32_t ByteArray::readUint32(size_t startPos) 
{
    uint32_t result = 0;
    for(int i = 0; i < 32; i += 7) 
    {
        uint8_t b = readFuint8(startPos);
        if(b < 0x80)
        {
            result |= ((uint32_t)b) << i;
            break;
        } 
        else 
        {
            result |= (((uint32_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

int64_t  ByteArray::readInt64(size_t startPos) 
{
    return DecodeZigzag64(readUint64(startPos));
}

uint64_t ByteArray::readUint64(size_t startPos) 
{
    uint64_t result = 0;
    for(int i = 0; i < 64; i += 7) 
    {
        uint8_t b = readFuint8(startPos);
        if(b < 0x80) 
        {
            result |= ((uint64_t)b) << i;
            break;
        } 
        else 
        {
            result |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

float    ByteArray::readFloat(size_t startPos) 
{
    uint32_t v = readFuint32(startPos);
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double   ByteArray::readDouble(size_t startPos) 
{
    uint64_t v = readFuint64(startPos);
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

std::string ByteArray::readStringF16(size_t startPos)
{
    uint16_t len = readFuint16(startPos);
    std::string buff;
    buff.resize(len);
    read(&buff[0], len, startPos);
    return buff;
}

std::string ByteArray::readStringF32(size_t startPos) 
{
    uint32_t len = readFuint32(startPos);
    std::string buff;
    buff.resize(len);
    read(&buff[0], len, startPos);
    return buff;
}

std::string ByteArray::readStringF64(size_t startPos) 
{
    uint64_t len = readFuint64(startPos);
    std::string buff;
    buff.resize(len);
    read(&buff[0], len, startPos);
    return buff;
}

std::string ByteArray::readStringVint(size_t startPos) 
{
    uint64_t len = readUint64(startPos);
    std::string buff;
    buff.resize(len);
    read(&buff[0], len, startPos);
    return buff;
}

void ByteArray::clear() 
{
    m_readPos = 0;
    m_writePos = 0;
    m_capacity = m_baseSize;
    Node* tmp = m_root->next;
    while(tmp) {
        m_writeBlock = tmp;
        tmp = tmp->next;
        delete m_writeBlock;
    }
    m_writeBlock = m_root;
    m_root->next = NULL;
}

void ByteArray::reset(size_t blockNo)
{

    if(blockNo < 1) return;
    Node* tmp = m_root;

    m_readPos = 0;
    m_writePos = 0;

    int reservedBlockNo = 0;
    while(tmp)
    {
        m_writeBlock = tmp;
        tmp = tmp->next;
        if(reservedBlockNo < blockNo)
            memset(m_writeBlock->ptr, 0, m_writeBlock->size);
        else 
            delete m_writeBlock;
        reservedBlockNo++;
    }

    m_writeBlock = m_root;
}

void ByteArray::refresh()
{
    reset(std::max(1, (int)((m_writePos / m_baseSize) / 2)));
}

void ByteArray::write(const void* buf, size_t size) 
{
    if(size == 0) return;
    addCapacityIfNotEnough(size);

    size_t posInCurBlock = m_writePos % m_baseSize;
    size_t curBlockFreeSize = m_baseSize - posInCurBlock;

    size_t writtenSize = 0;
    size_t leftSize = size;

    Node* cur = m_writeBlock;
    while(leftSize > 0) 
    {
        if(curBlockFreeSize >= leftSize)    // current block has enough memory
        {
            memcpy(cur->ptr + posInCurBlock, (const char*)buf + writtenSize, leftSize);
            leftSize = 0;
            if(cur->size == leftSize + posInCurBlock) 
            {   
                // current block full, jump to next block
                cur = cur->next;
            }

            break;
        } 

        // current block don't have enouth memory
        // fulfill current block
        memcpy(cur->ptr + posInCurBlock, (const char*)buf + writtenSize, curBlockFreeSize);

        writtenSize += curBlockFreeSize;
        leftSize -= curBlockFreeSize;

        // jump to next block
        cur = cur->next;
        curBlockFreeSize = cur->size;
        posInCurBlock = 0;
    }
}

void ByteArray::read(void* buf, size_t size, size_t startPos) 
{
    if(startPos < 0 || startPos > m_writePos) 
    {
        throw std::out_of_range("out of readable range");
    }

    if(size > getUnreadSize())
    {
        throw std::out_of_range("not enough size for read");
    }

    size_t posInCurBlock = startPos % m_baseSize;
    size_t curBlockUnreadSize = getCurBlockUnreadSize();

    size_t alreadyReadPos = 0;
    size_t leftSize = size;

    Node* cur = m_readBlock;
    while(leftSize > 0) 
    {
        // unread data is all in this block
        if(curBlockUnreadSize >= leftSize) 
        {
            memcpy((char*)buf + alreadyReadPos, cur->ptr + posInCurBlock, leftSize);
            alreadyReadPos += leftSize;

            if(cur->size == (posInCurBlock + leftSize)) 
            {
                cur = cur->next;
            }
            break;
        } 

        // some unread data still exists in behind blocks
        memcpy((char*)buf + alreadyReadPos, cur->ptr + posInCurBlock, curBlockUnreadSize);
        alreadyReadPos += curBlockUnreadSize;
        leftSize -= curBlockUnreadSize;

        cur = cur->next;
        curBlockUnreadSize = getCurBlockUnreadSize();
        posInCurBlock = 0;
    }
}

void ByteArray::read(void * buf, size_t size)
{   
    read(buf, size, m_readPos);
}

void ByteArray::addCapacityIfNotEnough(size_t size) 
{
    if(size <= 0 || getFreeSize() >= size) return;

    size_t addSize = size - getFreeSize();
    size_t count = ceil(1.5 * addSize / m_baseSize);
    Node* tmp = m_root;
    while(tmp->next) 
    {
        tmp = tmp->next;
    }

    Node* first = NULL;
    for(size_t i = 0; i < count; ++i) 
    {
        tmp->next = new Node(m_baseSize);
        if(first == NULL) {
            first = tmp->next;
        }
        tmp = tmp->next;
        m_capacity += m_baseSize;
    }
}

std::string ByteArray::toString()
{
    std::string str;
    str.resize(getUnreadSize());
    if(str.empty()) 
    {
        return str;
    }
    read(&str[0], str.size(), m_readPos);
    return str;
}

std::string ByteArray::toHexString() 
{
    std::string str = toString();
    std::stringstream ss;

    for(size_t i = 0; i < str.size(); ++i) 
    {
        if(i > 0 && i % 32 == 0) 
        {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
           << (int)(uint8_t)str[i] << " ";
    }

    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers
                                ,uint64_t len) 
{
    len = std::min(getUnreadSize(), len);
    if(len == 0) return 0;

    size_t posInCurrentBlock = m_readPos % m_baseSize;
    size_t unreadSizeInCurBlock = getCurBlockUnreadSize();

    size_t readAlreadySize = 0;
    size_t leftSize = len;

    Node* cur = m_readBlock;
    struct iovec iov;
    while(leftSize > 0) 
    {
        // all unread data is in current block
        if(unreadSizeInCurBlock >= leftSize) 
        {
            iov.iov_base = cur->ptr + posInCurrentBlock;
            iov.iov_len = len;
            leftSize = 0;
            buffers.push_back(iov);
            break;
        }

        // still other unread data exist in other block
        iov.iov_base = cur->ptr + posInCurrentBlock;
        iov.iov_len = unreadSizeInCurBlock;
        buffers.push_back(iov);

        cur = cur->next;

        leftSize -= unreadSizeInCurBlock;
        readAlreadySize += unreadSizeInCurBlock;

        posInCurrentBlock = 0;
        unreadSizeInCurBlock = std::min(m_baseSize, m_writePos - m_readPos - readAlreadySize); 
    }

    return len;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) 
{
    if(len == 0) return 0;

    addCapacityIfNotEnough(len);

    uint64_t leftSize = len;
    uint64_t writtenSize = 0;

    size_t posInCurBlock = m_writePos % m_baseSize;
    size_t freeSizeInCurBlock = m_writeBlock->size - posInCurBlock;

    struct iovec iov;
    Node* cur = m_writeBlock;
    while(leftSize > 0) 
    {
        if(freeSizeInCurBlock >= leftSize) 
        {
            iov.iov_base = cur->ptr + posInCurBlock;
            iov.iov_len = leftSize;
            leftSize = 0;
            
            buffers.push_back(iov);
            break;
        } 

        iov.iov_base = cur->ptr + posInCurBlock;
        iov.iov_len = freeSizeInCurBlock;
        buffers.push_back(iov);
        
        cur = cur->next;

        leftSize -= freeSizeInCurBlock;
        writtenSize += freeSizeInCurBlock;

        freeSizeInCurBlock = cur->size;
        posInCurBlock = 0;
        
    }
    return len;
}

void ByteArray::hasRead(size_t size)
{
    if(size <= 0 || size > getUnreadSize())
        throw std::out_of_range("out of read range");

    size_t tmpPos = m_readPos;
    m_readPos += size;

    while(tmpPos < m_readPos)
    {
        m_readBlock = m_readBlock->next;
        tmpPos += m_baseSize;
    }
}

void ByteArray::hasWritten(size_t size)
{
    if(size <= 0 || size > getFreeSize())
        throw std::out_of_range("out of write range");

    size_t tmpPos = m_writePos;
    m_writePos += size;

    while(tmpPos < m_writePos)
    {
        m_writeBlock = m_writeBlock->next;
        tmpPos += m_baseSize;
    }
}



#endif