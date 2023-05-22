#include <iostream>

#include "LogFileOutputter.h"
#include "Logger.h"

LogFileOutputter::LogFileOutputter(const std::string filename, std::string pattern)
: LogOutputter(pattern), m_fileName(filename) 
{
    if(!filename.empty()) reopen();
}

void LogFileOutputter::log(Logger::ptr logger, LogLevel::Level level, LogItem::ptr item) 
{
    if(level >= m_level) 
    {
        uint64_t now = item->getTime();
        if(now >= (m_lastTime + 3)) 
        {
            reopen();
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);
        //if(!(m_filestream << m_formatter->format(logger, level, item))) {
        if(!m_formatter->format(m_fileStream, logger, level, item)) 
        {
            std::cout << "error" << std::endl;
        }
    }
}

std::string LogFileOutputter::toYamlString() 
{
    // MutexType::Lock lock(m_mutex);
    // YAML::Node node;
    // node["type"] = "FileLogOutputter";
    // node["file"] = m_filename;
    // if(m_level != LogLevel::UNKNOW) {
    //     node["level"] = LogLevel::ToString(m_level);
    // }
    // if(m_hasFormatter && m_formatter) {
    //     node["formatter"] = m_formatter->getPattern();
    // }
    // std::stringstream ss;
    // ss << node;
    // return ss.str();

    return "";
}

bool LogFileOutputter::reopen() 
{
    bool retp = true;
    MutexType::Lock lock(m_mutex);
    if(m_fileStream) 
    {
        m_fileStream.close();
    }

    retp =  FileSystem::OpenForWrite(m_fileStream, m_fileName, std::ios::app);
    if(!retp)   LOG_ERROR(LOG_ROOT()) << "open file: " << m_fileName << "failed";
    return retp;
}

bool LogFileOutputter::reopen(const std::string fileName)
{
    m_fileName = fileName;
    reopen();
}

