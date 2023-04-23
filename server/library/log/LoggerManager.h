#ifndef LOG_LOGGERMANAGER_H
#define LOG_LOGGERMANAGER_H

#include <map>

#include "../Singleton.h"
#include "../Mutex.h"
class Logger;
// logger manager class
class LoggerManager {
public:
    typedef Spinlock MutexType;
    LoggerManager();

    std::shared_ptr<Logger> getLogger(const std::string& name);

    void init();

    std::shared_ptr<Logger> getRoot() const { return m_root;}

    std::string toYamlString();
	
private:
    MutexType m_mutex;
    std::map<std::string, std::shared_ptr<Logger>> m_loggers;
    /// main logger
    std::shared_ptr<Logger> m_root;
};

// singleton
typedef Singleton<LoggerManager> LoggerMgr;

#endif