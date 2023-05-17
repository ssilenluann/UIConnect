#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <memory>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

#include "../../Noncopiable.h"

class ByteArray: Noncopyable
{
public:
    typedef std::shared_ptr<ByteArray> ptr;

    struct Node 
    {
        Node(size_t s);
        Node();
        ~Node();

        char* ptr;
        Node* next;
        size_t size;
    };

    ByteArray(size_t base_size = 4096);

    virtual ~ByteArray();

    static uint32_t EncodeZigzag32(const int32_t& v);
    static uint64_t EncodeZigzag64(const int64_t& v);
    static int32_t DecodeZigzag32(const uint32_t& v);
    static int64_t DecodeZigzag64(const uint64_t& v);

    void writeFint8  (int8_t value);
    void writeFuint8 (uint8_t value);
    void writeFint16 (int16_t value);
    void writeFuint16(uint16_t value);
    void writeFint32 (int32_t value);
    void writeFuint32(uint32_t value);
    void writeFint64 (int64_t value);
    void writeFuint64(uint64_t value);
    void writeInt32  (int32_t value);
    void writeUint32 (uint32_t value);
    void writeInt64  (int64_t value);
    void writeUint64 (uint64_t value);
    void writeFloat  (float value);
    void writeDouble (double value);
    void writeStringF16(const std::string& value);
    void writeStringF32(const std::string& value);
    void writeStringF64(const std::string& value);
    void writeStringVint(const std::string& value);
    void writeStringWithoutLength(const std::string& value);

    int8_t   readFint8(size_t startPos);
    uint8_t  readFuint8(size_t startPos);
    int16_t  readFint16(size_t startPos);
    uint16_t readFuint16(size_t startPos);
    int32_t  readFint32(size_t startPos);
    uint32_t readFuint32(size_t startPos);
    int64_t  readFint64(size_t startPos);
    uint64_t readFuint64(size_t startPos);
    int32_t  readInt32(size_t startPos);
    uint32_t readUint32(size_t startPos);
    int64_t  readInt64(size_t startPos);
    uint64_t readUint64(size_t startPos);
    float    readFloat(size_t startPos);
    double   readDouble(size_t startPos);
    std::string readStringF16(size_t startPos);
    std::string readStringF32(size_t startPos);
    std::string readStringF64(size_t startPos);
    std::string readStringVint(size_t startPos);

    void write(const void* buf, size_t size);
    void read(void* buf, size_t size, size_t startPos);
    void read(void* buf, size_t size);

    void clear();
    void reset(size_t reserveBlockNo = 1);
    void refresh();

    bool isLittleEndian() const;
    void setIsLittleEndian(bool val);
    
    inline size_t getBaseSize() const { return m_baseSize;}
    inline size_t getSize() const { return m_writePos;}
    inline size_t getFreeSize() const { return m_capacity - m_writePos;}
    inline size_t getUnreadSize() const { return m_writePos - m_readPos;}
    inline bool empty() const { return m_writePos == 0;}
    
    std::string toString();
    std::string toHexString();

    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len);
    void hasRead(size_t size);
    void hasWritten(size_t size);

protected:
    inline size_t getCurBlockUnreadSize() const { return std::min(m_writePos - m_readPos, m_baseSize);}

    void addCapacityIfNotEnough(size_t size);

protected:
    
    size_t m_baseSize; /// memory block size
    size_t m_writePos; /// current write position
    size_t m_readPos; /// current write position
    size_t m_capacity; /// current capacity
    int8_t m_endian; /// endian, default big endian
    Node* m_root; /// first memory block pointer
    Node* m_writeBlock; /// current write memroy block
    Node* m_readBlock; /// current read memroy block
};

#endif