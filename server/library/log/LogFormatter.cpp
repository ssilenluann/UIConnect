#ifndef LOG_LOGFORMATTER_CPP
#define LOG_LOGFORMATTER_CPP

#include "LogFormatter.h"

LogFormatter::LogFormatter(std::string &pattern): m_pattern(pattern)
{
}

std::string LogFormatter::format(std::shared_ptr<LogItem> event)
{
    std::stringstream ss;
    for(auto& item: m_items)
    {
        item->format(ss, event);
    }

    return ss.str();
}

void LogFormatter::init()
{
    std::vector<std::pair<std::string, int>> vec;
    for(size_t i = 0; )
}

void LogFormatter::FormatItem::format(std::ostream& os,std::shared_ptr<LogItem> event)
{
    return std::string();
}
#endif