#ifndef UTILS_STRINGUTIL_H
#define UTILS_STRINGUTIL_H

#include <cxxabi.h>
#include <string>
class StringUtil
{
public:

    template<class T>
    static const char* TypeToName()
    {
        static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }

    static void ToLower(std::string& str);

};


#endif