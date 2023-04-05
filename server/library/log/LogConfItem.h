#ifndef LOG_LOGCONFITEM_H
#define LOG_LOGCONFITEM_H

#include "Logger.h"
#include "outputter/LogFileOutputter.h"
#include "outputter/LogStdoutOutputter.h"
#include "../config/Config.h"

#include <map>
#include <vector>
#include <string>

class LogConfItem
{
public:

    bool operator==(const LogConfItem& rhs) const
    {
        return name == rhs.name && level == rhs.level && pattern == rhs.pattern && outputters == rhs.outputters;
    }

public:
    class Outputter
    {
    
    public:

        bool operator==(const Outputter& rhs) const
        {
            return type == rhs.type && file == rhs.file;
        }
    public:
        std::string type;
        std::string file;
    };

public:
    std::string name;
    std::string level;
    std::string pattern;
    std::vector<Outputter> outputters;
};

template<>
class ItemConverter<LogConfItem, std::string>
{
public:
    std::string operator()(const LogConfItem& log)
    {
        YAML::Node node;
        node["name"] = log.name;
        node["level"] = log.level;
        node["pattern"] = log.pattern;

        YAML::Node outputters(YAML::NodeType::Sequence);
        std::ostringstream ss;
        for(auto& op: log.outputters)
        {
            YAML::Node outputter;
            if(!op.type.empty()) outputter["type"] = op.type;
            if(!op.file.empty()) outputter["file"] = op.file;
            
            ss.str("");
            ss << node;
            outputters.push_back(ss.str());
        }
        ss.str("");
        ss << outputters;
        node["outputters"] = ss.str();

        ss.str("");
        ss << node;
        return ss.str();
    }
};

template<>
class ItemConverter<std::string, LogConfItem>
{
public:
    LogConfItem operator()(const std::string& str)
    {
        LogConfItem item;
        YAML::Node node = YAML::Load(str);
        if(!node.IsMap())
            return item;

        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) 
        {
            std::string key = it->first.as<std::string>();
            
            if(key == "name") item.name = it->second.as<std::string>();
            if(key == "level")  item.level = it->second.as<std::string>();
            if(key == "pattern")  item.pattern = it->second.as<std::string>();
            if(key == "outputters")
            {
                if(!it->second.IsSequence())    continue;
                for(int i = 0; i < it->second.size(); ++i)
                {
                    if(!it->second[i].IsMap())  continue;

                    LogConfItem::Outputter outputter;
                    for(YAML::const_iterator it2 = it->second[i].begin(); it2 != it->second[i].end(); ++it2)
                    {
                        
                        key = it2->first.as<std::string>();
                        if(key == "type") outputter.type = it2->second.as<std::string>();
                        if(key == "file") outputter.file = it2->second.as<std::string>();
                    }
                    item.outputters.push_back(outputter);
                }
            }

        }

        return item;

    }
};

// before main func starts, set up log config 
ConfigItem<std::vector<LogConfItem>>::ptr g_log_confs =
    Config::SearchOrAdd("logs", std::vector<LogConfItem>(), "logs config");

class LogIniter
{
public:
    LogIniter()
    {
        g_log_confs->addListener([](const std::vector<LogConfItem>& old_value, const std::vector<LogConfItem>& new_value){
            struct Level_Map
            {
                std::string level_conf;
                LogLevel::Level level;
            } level[] = 
            {
                {"unknown", LogLevel::Level::UNKNOWN},
                {"debug", LogLevel::Level::DEBUG},
                {"info", LogLevel::Level::INFO},
                {"warn", LogLevel::Level::WARN},
                {"error", LogLevel::Level::ERROR},
                {"fatal", LogLevel::Level::FATAL}
            };

            LOG_INFO(LOG_ROOT()) << "on_logger_conf_changed";

            // delete old log outputter
            for(auto& val: old_value)
            {
                auto logger = LOG_NAME(val.name);
                logger->setLevel(LogLevel::Level::UNKNOWN);
                logger->clearOutputters();
            }

            for(const auto& val: new_value)
            {
                // get or create logger
                auto logger = LOG_NAME(val.name);

                // set level
                for(int i = 0; i < sizeof(level) / sizeof(Level_Map); i++)
                {
                    if(level[i].level_conf == val.level)
                    {
                        logger->setLevel(level[i].level);
                        continue;
                    }
                }
                
                for(const auto& op: val.outputters)
                {
                    if(op.type == "FileLogOutputter")
                    {
                        logger->addOutputter(std::make_shared<LogFileOutputter>(FileSystem::WorkPath() + op.file, val.pattern));
                    }
                    else if(op.type == "StdoutOutputter")
                    {
                        logger->addOutputter(std::make_shared<LogStdoutOutputter>(val.pattern));
                    }
                    else
                    {
                        LOG_INFO(LOG_ROOT()) << "unkonwn log outputter type";
                    }
                }
                
            }

        });
    }
};

static LogIniter g_logger_initer;
#endif