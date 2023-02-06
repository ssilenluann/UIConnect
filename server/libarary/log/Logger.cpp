#ifndef LOG_LOGGER_CPP
#define LOG_LOGGER_CPP

#include "Logger.h"   

Logger::Logger(const std::string& name)
:m_name(name), m_level(LogLevel::Level::DEBUG)
{

}

void Logger::log(LogLevel::Level level, std::shared_ptr<LogItem> event)
{
    if(level >= m_level)
    {
        for(auto& appender: m_appenders)
        {
            appender->log(level, event);
        }
    }
}
void Logger::debug(std::shared_ptr<LogItem> event)
{
    log(LogLevel::Level::DEBUG, event);
}
void Logger::info(std::shared_ptr<LogItem> event)
{
    log(LogLevel::Level::INFO, event);
}
void Logger::warn(std::shared_ptr<LogItem> event)
{
    log(LogLevel::Level::WARN, event);
}
void Logger::error(std::shared_ptr<LogItem> event)
{
    log(LogLevel::Level::ERROR, event);
}
void Logger::fatal(std::shared_ptr<LogItem> event)
{
    log(LogLevel::Level::FATAL, event);
}

void Logger::addAppender(std::shared_ptr<LogOutputter>& appender)
{
    m_appenders.push_back(appender);
}

void Logger::delAppender(std::shared_ptr<LogOutputter>& appender)
{
    for(auto it = m_appenders.begin(); it != m_appenders.end(); it++)
    {
        if(*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

#endif