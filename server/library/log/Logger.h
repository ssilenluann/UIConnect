#ifndef LOG_LOGGER_H
#define LOG_LOGGER_H

#include <string>
#include <memory>
#include <list>

#include "LogLevel.h"
#include "LogItem.h"
#include "../Mutex.h"

class LogOutputter;
class LogFormatter;
class Logger : public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    Logger(const std::string& name = "root");

    void log(LogLevel::Level level, LogItem::ptr item);
    void debug(LogItem::ptr item);
    void info(LogItem::ptr item);
    void warn(LogItem::ptr item);
    void error(LogItem::ptr item);
    void fatal(LogItem::ptr item);

    void addOutputter(std::shared_ptr<LogOutputter> outputter);
    void delOutputter(std::shared_ptr<LogOutputter> outputter);
    void clearOutputters();

    LogLevel::Level getLevel() const { return m_level;}

    void setLevel(LogLevel::Level val) { m_level = val;}

    const std::string& getName() const { return m_name;}

    void setFormatter(std::shared_ptr<LogFormatter> val);
    void setFormatter(const std::string& val);
    std::shared_ptr<LogFormatter> getFormatter();

    std::string toYamlString();
private:
    // logger name
    std::string m_name;
    // log level
    LogLevel::Level m_level;
    // Mutex
    MutexType m_mutex;
    // outputters
    std::list<std::shared_ptr<LogOutputter>> m_outputters;
    // formatter
    std::shared_ptr<LogFormatter> m_formatter;
    // logger
    Logger::ptr m_root;
};
#endif