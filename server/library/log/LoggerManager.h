#ifndef LOG_LOGGERMANAGER_H
#define LOG_LOGGERMANAGER_H

#include <map>

#include "Logger.h"
#include "../Singleton.h"
#include "../Mutex.h"

// logger manager class
class LoggerManager {
public:
    typedef Spinlock MutexType;
    LoggerManager();

    Logger::ptr getLogger(const std::string& name);

    void init();

    Logger::ptr getRoot() const { return m_root;}

    std::string toYamlString();
	
private:
    MutexType m_mutex;
    std::map<std::string, Logger::ptr> m_loggers;
    /// main logger
    Logger::ptr m_root;
};

// singleton
typedef Singleton<LoggerManager> LoggerMgr;

#endif