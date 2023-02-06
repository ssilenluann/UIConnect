#ifndef LOG_LOGGERMANAGER_CPP
#define LOG_LOGGERMANAGER_CPP

#include <memory>

#include "LoggerManager.h"
#include "LogOutputter.h"
#include "outputter/LogStdoutOutputter.h"

LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addOutputter(LogOutputter::ptr(new LogStdoutOutputter));

    m_loggers[m_root->m_name] = m_root;

    init();
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) {
        return it->second;
    }

    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

std::string LoggerManager::toYamlString() {
    // MutexType::Lock lock(m_mutex);
    // YAML::Node node;
    // for(auto& i : m_loggers) {
    //     node.push_back(YAML::Load(i.second->toYamlString()));
    // }
    // std::stringstream ss;
    // ss << node;
    // return ss.str();
    return "";
}

void LoggerManager::init() {
}

#endif