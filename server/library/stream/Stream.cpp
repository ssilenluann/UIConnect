#include "Stream.h"

int Stream::readFixedSize(void *buffer, size_t length)
{
    size_t offset = 0;
    int64_t left = length;
    while(left > 0)
    {
        int64_t len = read((char*)buffer + offset, left);
        if(len <= 0)    return len;

        offset += len;
        left -= len;   
    }

    return length;
}

int Stream::readFixedSize(ByteArray::ptr ba, size_t length)
{
    int64_t left = length;
    while(left > 0) {
        int64_t len = read(ba, left);
        if(len <= 0) {
            return len;
        }
        left -= len;
    }
    return length;
}

int Stream::wirteFixedSize(void *buffer, size_t length)
{
    size_t offset = 0;
    int64_t left = length;
    while(left > 0) {
        int64_t len = write((const char*)buffer + offset, left);
        if(len <= 0) {
            return len;
        }
        offset += len;
        left -= len;
    }
    return length;
}

int Stream::wirteFixedSize(ByteArray::ptr ba, size_t length)
{
    int64_t left = length;
    while(left > 0) {
        int64_t len = write(ba, left);
        if(len <= 0) {
            return len;
        }
        left -= len;
    }
    return length;
}
