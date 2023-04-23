#ifndef LOG_LOGOUTPUTTER_H
#define LOG_LOGOUTPUTTER_H

#include <memory>

#include "LogLevel.h"
#include "LogItem.h"
#include "Logger.h"
#include "LogFormatter.h"
#include "../Mutex.h"

// Persistence
class LogOutputter
{
	friend class Logger;
public:
    typedef std::shared_ptr<LogOutputter> ptr;

    LogOutputter(std::string pattern = "");
    virtual ~LogOutputter(){}

    virtual void log(Logger::ptr logger, LogLevel::Level level, LogItem::ptr item) = 0;
    virtual std::string toYamlString() = 0;

    void setFormatter(LogFormatter::ptr formatter);
    LogFormatter::ptr getFormatter();
    virtual void setLevel(LogLevel::Level level) { m_level = level;}

protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    bool m_hasFormatter = false;
    // Mutex
    Logger::MutexType m_mutex;
    // 日志格式器
    LogFormatter::ptr m_formatter;
}; 

#endif