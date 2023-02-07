#ifndef LOG_LOGGER_CPP
#define LOG_LOGGER_CPP

#include <iostream>

#include "Logger.h"   
#include "LogOutputter.h"  
#include "LogFormatter.h" 

Logger::Logger(const std::string& name)
:m_name(name),m_level(LogLevel::DEBUG) 
{
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::setFormatter(LogFormatter::ptr val)
{
    MutexType::Lock lock(m_mutex);
    m_formatter = val;

    for(auto& i : m_outputters) 
    {
        MutexType::Lock ll(i->m_mutex);
        if(!i->m_hasFormatter) 
        {
            i->m_formatter = m_formatter;
        }
    }
}

void Logger::setFormatter(const std::string& val) 
{
    std::cout << "---" << val << std::endl;
    LogFormatter::ptr new_val(new LogFormatter(val));
    if(new_val->isError()) 
    {
        std::cout << "Logger setFormatter name=" << m_name
                  << " value=" << val << " invalid formatter"
                  << std::endl;
        return;
    }
    //m_formatter = new_val;
    setFormatter(new_val);
}

std::string Logger::toYamlString() 
{
    // MutexType::Lock lock(m_mutex);
    // YAML::Node node;
    // node["name"] = m_name;
    // if(m_level != LogLevel::UNKNOW) {
    //     node["level"] = LogLevel::ToString(m_level);
    // }
    // if(m_formatter) {
    //     node["formatter"] = m_formatter->getPattern();
    // }

    // for(auto& i : m_outputters) {
    //     node["outputters"].push_back(YAML::Load(i->toYamlString()));
    // }
    // std::stringstream ss;
    // ss << node;
    // return ss.str();
    return "";
}


LogFormatter::ptr Logger::getFormatter() 
{
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

void Logger::addOutputter(LogOutputter::ptr outputter) 
{
    MutexType::Lock lock(m_mutex);
    if(!outputter->getFormatter()) {
        MutexType::Lock ll(outputter->m_mutex);
        outputter->m_formatter = m_formatter;
    }
    m_outputters.push_back(outputter);
}

void Logger::delOutputter(LogOutputter::ptr outputter) 
{
    MutexType::Lock lock(m_mutex);
    for(auto it = m_outputters.begin();
            it != m_outputters.end(); ++it) {
        if(*it == outputter) {
            m_outputters.erase(it);
            break;
        }
    }
}

void Logger::clearOutputters() 
{
    MutexType::Lock lock(m_mutex);
    m_outputters.clear();
}

void Logger::log(LogLevel::Level level, LogItem::ptr item) 
{
    if(level >= m_level) 
    {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if(!m_outputters.empty()) 
        {
            for(auto& i : m_outputters) 
            {
                i->log(self, level, item);
            }
        } 
        else if(m_root) 
        {
            m_root->log(level, item);
        }
    }
}

void Logger::debug(LogItem::ptr item) 
{
    log(LogLevel::DEBUG, item);
}

void Logger::info(LogItem::ptr item) 
{
    log(LogLevel::INFO, item);
}

void Logger::warn(LogItem::ptr item) 
{
    log(LogLevel::WARN, item);
}

void Logger::error(LogItem::ptr item) 
{
    log(LogLevel::ERROR, item);
}

void Logger::fatal(LogItem::ptr item) 
{
    log(LogLevel::FATAL, item);
}

#endif