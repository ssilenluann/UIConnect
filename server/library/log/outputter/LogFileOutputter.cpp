#ifndef LOG_LOGFILEOUTPUTTER_CPP
#define LOG_LOGFILEOUTPUTTER_CPP
#include <iostream>

#include "LogFileOutputter.h"

LogFileOutputter::LogFileOutputter(const std::string& filename)
    :m_fileName(filename) {
    reopen();
}

void LogFileOutputter::log(Logger::ptr logger, LogLevel::Level level, LogItem::ptr item) {
    if(level >= m_level) {
        uint64_t now = item->getTime();
        if(now >= (m_lastTime + 3)) {
            reopen();
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);
        //if(!(m_filestream << m_formatter->format(logger, level, item))) {
        if(!m_formatter->format(m_fileStream, logger, level, item)) {
            std::cout << "error" << std::endl;
        }
    }
}

std::string LogFileOutputter::toYamlString() {
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

bool LogFileOutputter::reopen() {
    MutexType::Lock lock(m_mutex);
    if(m_fileStream) {
        m_fileStream.close();
    }
    return FileSystem::OpenForWrite(m_fileStream, m_fileName, std::ios::app);
}

#endif