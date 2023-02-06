#ifndef LOG_LOGGER_H
#define LOG_LOGGER_H

#include <string>
#include <memory>
#include <list>

#include "LogLevel.h"
#include "LogItem.h"
#include "LogOutputter.h"
class Logger : public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    Logger(const std::string& name = "root");

    void log(LogLevel::Level level, LogItem::ptr event);
    void debug(LogItem::ptr event);
    void info(LogItem::ptr event);
    void warn(LogItem::ptr event);
    void error(LogItem::ptr event);
    void fatal(LogItem::ptr event);

    void addAppender(LogOutputter::ptr appender);
    void delAppender(LogOutputter::ptr appender);
    void clearAppenders();

    LogLevel::Level getLevel() const { return m_level;}

    void setLevel(LogLevel::Level val) { m_level = val;}

    const std::string& getName() const { return m_name;}

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter();

    std::string toYamlString();
private:
    // logger name
    std::string m_name;
    // log level
    LogLevel::Level m_level;
    // Mutex
    MutexType m_mutex;
    // appenders
    std::list<LogOutputter::ptr> m_appenders;
    // formatter
    LogFormatter::ptr m_formatter;
    // logger
    Logger::ptr m_root;
};
#endif