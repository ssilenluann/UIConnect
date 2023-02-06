#ifndef LOG_LOGSTDOUTOUTPUTTER_CPP
#define LOG_LOGSTDOUTOUTPUTTER_CPP

#include "LogStdoutOutputter.h"

#include <iostream>
void LogStdoutOutputter::log(LogLevel::Level level, std::shared_ptr<LogItem> event)
{
    if(level >= m_level)
        std::cout << m_formatter->format(event);
}
#endif