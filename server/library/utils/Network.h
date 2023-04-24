#ifndef UTILS_NETWORK_H
#define UTILS_NETWORK_H

class Network
{
public:
    template<class T>
    static T CreateMask(uint32_t bits)
    {
        return (1 << (sizeof(T) * 8 - bits)) - 1;
    }

    template <class T>
    static uint32_t countBytes(T value)
    {
        uint32_t result = 0;
        for(; value; ++result)
        {
            value &= value - 1;
        }

        return result;
    }
};
#endif