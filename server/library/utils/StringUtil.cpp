#include "StringUtil.h"
#include <algorithm>
#include <cctype>

void StringUtil::ToLower(std::string & str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::isalpha(c)? std::tolower(c): c;
    });    
}



