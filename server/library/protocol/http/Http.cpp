#ifndef PROTOCOL_HTTP_HTTP_CPP
#define PROTOCOL_HTTP_HTTP_CPP
#include "Http.h"
#include <string.h>
HttpMethod Http::String2HttpMethod(const std::string & str)
{
#define XX(num, name, string)\
    if(strncmp(#string, str.c_str(), strlen(#string)) == 0)\
        return HttpMethod::name;
    HTTP_METHOD_MAP(XX);
#undef XX

    return HttpMethod::INVALID_METHOD;
}
HttpMethod Http::String2HttpMethod(const char *str)
{
#define XX(num, name, string)\
    if(strncmp(#string, str, strlen(#string)) == 0) \
        return HttpMethod::name; 
    HTTP_METHOD_MAP(XX);
#undef XX
    
    return HttpMethod::INVALID_METHOD;
}

static const char* s_method_string[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
};
const char *Http::HttpMethod2String(HttpMethod &method)
{
    uint32_t idx = (uint32_t)method;
    if(idx >= sizeof(s_method_string) / sizeof(s_method_string[0]))
        return "<unknown>";

    return s_method_string[idx];
}
const char *Http::HttpStatus2String(HttpStatus &status)
{
    switch(status)
    {
#define XX(code, name, msg) \ 
        case HttpStatus::name: \ 
            return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
        default:
            return "unknown";
    }

}
#endif