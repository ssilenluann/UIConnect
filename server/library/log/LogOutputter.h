#ifndef LOG_LOGADDPANDER_H
#define LOG_LOGADDPANDER_H

#include <memory>

#include "LogLevel.h"
#include "LogItem.h"
#include "LogFormatter.h"
// Persistence
class LogOutputter
{
	friend class Logger;
public:
    virtual ~LogOutputter(){}

    virtual void log(LogLevel::Level level, std::shared_ptr<LogItem> item) = 0;

    void setFormatter(std::shared_ptr<LogFormatter>& formatter) { m_formatter = formatter;}
    std::shared_ptr<LogFormatter> getFormatter(){ return m_formatter;}

protected:
    LogLevel::Level m_level;
    std::shared_ptr<LogFormatter> m_formatter;
}; 

#endif