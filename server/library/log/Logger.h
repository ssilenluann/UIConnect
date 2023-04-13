#ifndef LOG_LOGGER_H
#define LOG_LOGGER_H

#include <string>
#include <memory>
#include <list>

#include "LogLevel.h"
#include "LogItem.h"
#include "LoggerManager.h"
#include "../Mutex.h"
#include "../utils/ThreadUtil.h"

#pragma message("#include Logger.h")

#define LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        LogItemWrap(LogItem::ptr(new LogItem(logger, level, \
                        __FILE__, __LINE__, 0, ThreadUtil::GetThreadId(),\
                0, time(0), ""))).getSS()

#define LOG_DEBUG(logger) LOG_LEVEL(logger, LogLevel::DEBUG)

#define LOG_INFO(logger) LOG_LEVEL(logger, LogLevel::INFO)

#define LOG_WARN(logger) LOG_LEVEL(logger, LogLevel::WARN)

#define LOG_ERROR(logger) LOG_LEVEL(logger, LogLevel::ERROR)

#define LOG_FATAL(logger) LOG_LEVEL(logger, LogLevel::FATAL)

#define LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        LogItemWrap(LogItem::ptr(new LogItem(logger, level, \
                        __FILE__, __LINE__, 0, ThreadUtil::GetThreadId(),\
                0, time(0), ""))).getItem()->format(fmt, __VA_ARGS__)

#define LOG_FMT_DEBUG(logger, fmt, ...) LOG_FMT_LEVEL(logger, LogLevel::DEBUG, fmt, __VA_ARGS__)

#define LOG_FMT_INFO(logger, fmt, ...)  LOG_FMT_LEVEL(logger, LogLevel::INFO, fmt, __VA_ARGS__)

#define LOG_FMT_WARN(logger, fmt, ...)  LOG_FMT_LEVEL(logger, LogLevel::WARN, fmt, __VA_ARGS__)

#define LOG_FMT_ERROR(logger, fmt, ...) LOG_FMT_LEVEL(logger, LogLevel::ERROR, fmt, __VA_ARGS__)

#define LOG_FMT_FATAL(logger, fmt, ...) LOG_FMT_LEVEL(logger, LogLevel::FATAL, fmt, __VA_ARGS__)

#define LOG_ROOT() LoggerMgr::GetInstance()->getRoot()

#define LOG_NAME(name) LoggerMgr::GetInstance()->getLogger(name)

#define LOG_ASSERT(x) \ 
    if(!(x)) \
    { \
        LOG_ERROR(LOG_ROOT()) << "ASSERT: " #x \
            << "\nbacktrace:\n" \ 
            << ThreadUtil::Backtrace2String(); \
    }

#define LOG_ASSERT_W(x, w) \ 
    if(!(x)) \
    { \
        LOG_ERROR(LOG_ROOT()) << "ASSERT: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \ 
            << ThreadUtil::Backtrace2String(); \
    }

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