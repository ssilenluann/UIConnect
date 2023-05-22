#include "LogOutputter.h"

LogOutputter::LogOutputter(std::string pattern)
{
    if(!pattern.empty())
    {
        setFormatter(std::make_shared<LogFormatter>(pattern));
    }
}

void LogOutputter::setFormatter(LogFormatter::ptr val) 
{
    Logger::MutexType::Lock lock(m_mutex);
    m_formatter = val;
    m_hasFormatter =  m_formatter != nullptr;
}

LogFormatter::ptr LogOutputter::getFormatter() 
{
    Logger::MutexType::Lock lock(m_mutex);
    return m_formatter;
}
