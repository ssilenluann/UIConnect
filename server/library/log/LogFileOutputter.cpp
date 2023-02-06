#ifndef LOG_LOGFILEOUTPUTTER_CPP
#define LOG_LOGFILEOUTPUTTER_CPP
#include "LogFileOutputter.h"
LogFileOutputter::LogFileOutputter(const std::string& fileName)
: m_fileName(fileName)
{

}

void LogFileOutputter::log(LogLevel::Level level, std::shared_ptr<LogItem> item)
{
    if(level >= m_level)
        m_fileStream << m_formatter->format(item);
}

bool LogFileOutputter::reopen()
{
    if(m_fileStream)    m_fileStream.close();
    m_fileStream.open(m_fileName);
    return !!m_fileStream;
}

#endif