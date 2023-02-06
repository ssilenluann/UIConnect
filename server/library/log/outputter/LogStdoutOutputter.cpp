#ifndef LOG_LOGSTDOUTOUTPUTTER_CPP
#define LOG_LOGSTDOUTOUTPUTTER_CPP

#include "LogStdoutOutputter.h"

#include <iostream>
void LogStdoutOutputter::log(LogLevel::Level level, std::shared_ptr<LogItem> item)
{
    if(level >= m_level)
        std::cout << m_formatter->format(item);
}

void StdoutLogOutputter::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogItem::ptr item) {
    if(level >= m_level) {
        MutexType::Lock lock(m_mutex);
        m_formatter->format(std::cout, logger, level, item);
    }
}

std::string StdoutLogOutputter::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogOutputter";
    if(m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_hasFormatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}
#endif