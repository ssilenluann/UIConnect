#ifndef LOG_LOGITEM_H
#define LOG_LOGITEM_H

#include <memory>
#include <sstream>

#include "LogLevel.h"

class Logger;
/**
 * @brief class for log content info
 */
class LogItem {
public:
    typedef std::shared_ptr<LogItem> ptr;

    LogItem(std::shared_ptr<Logger> logger, 
        LogLevel::Level level
        ,const char* file, int32_t line, uint32_t elapse
        ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
        ,const std::string& thread_name);

    const char* getFile() const { return m_file;}

    int32_t getLine() const { return m_line;}

    uint32_t getElapse() const { return m_elapse;}

    uint32_t getThreadId() const { return m_threadId;}
	
	uint32_t getFiberId() const { return m_fiberId;}

    uint64_t getTime() const { return m_time;}

    const std::string& getThreadName() const { return m_threadName;}

    std::string getContent() const { return m_ss.str();}

    std::shared_ptr<Logger> getLogger() const { return m_logger;}

    LogLevel::Level getLevel() const { return m_level;}

    std::stringstream& getSS() { return m_ss;}

    void format(const char* fmt, ...);

    void format(const char* fmt, va_list al);
    
private:
    // file name
    const char* m_file = nullptr;
    // line name
    int32_t m_line = 0;
    // elapsed time(ms) from program started to now
    uint32_t m_elapse = 0;
    // thread id
    uint32_t m_threadId = 0;
	// fiber id
	uint32_t m_fiberId = 0;
    // time stamp
    uint64_t m_time = 0;
    // thread name
    std::string m_threadName;
    // string stream for log content
    std::stringstream m_ss;
    // logger
    std::shared_ptr<Logger> m_logger;
    // log level
    LogLevel::Level m_level;
};

class LogItemWrap
{
public:
    LogItemWrap(LogItem::ptr e);
    ~LogItemWrap();

    LogItem::ptr getItem() const { return m_item;}
	
	/**
	* @brief get string stream
	*/
    std::stringstream& getSS();
private:
    LogItem::ptr m_item;
};
#endif