#ifndef LOG_LOGITEM_CPP
#define LOG_LOGITEM_CPP

#include <stdarg.h>

#include "LogItem.h"
#include "Logger.h"

LogItemWrap::LogItemWrap(LogItem::ptr e)
:m_item(e) 
{
}

LogItemWrap::~LogItemWrap() 
{
    m_item->getLogger()->log(m_item->getLevel(), m_item);
}

std::stringstream& LogItemWrap::getSS() 
{
    return m_item->getSS();
}

LogItem::LogItem(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name)
:m_file(file),m_line(line),m_elapse(elapse),m_threadId(thread_id),m_fiberId(fiber_id)
,m_time(time),m_threadName(thread_name),m_logger(logger),m_level(level) 
{
}

void LogItem::format(const char* fmt, ...) 
{
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogItem::format(const char* fmt, va_list al) 
{
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

#endif