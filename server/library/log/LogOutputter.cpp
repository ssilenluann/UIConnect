#ifndef LOG_LOGOUTPUTTER_CPP
#define LOG_LOGOUTPUTTER_CPP

#include "LogOutputter.h"

void LogOutputter::setFormatter(LogFormatter::ptr& val) {
    Logger::MutexType::Lock lock(m_mutex);
    m_formatter = val;
    if(m_formatter) {
        m_hasFormatter = true;
    } else {
        m_hasFormatter = false;
    }
}

LogFormatter::ptr LogOutputter::getFormatter() {
    Logger::MutexType::Lock lock(m_mutex);
    return m_formatter;
}


#endif