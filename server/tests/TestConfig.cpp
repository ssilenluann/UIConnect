#include "log/Logger.h"
#include "config/Config.h"
#include "ProgramOpt.h"
#include <yaml-cpp/yaml.h>

#include <vector>
#include <map>
#include <iostream>
#include <string>

class ConfLogItem
{
public:
    ConfLogItem(){}
    ConfLogItem(const ConfLogItem& cfg)
    {
        name = cfg.name;
        level = cfg.level;
        for(auto& op: cfg.outputters)
            outputters.push_back(op);
    }

    ConfLogItem& operator=(const ConfLogItem& rhs)
    {
        if(this != &rhs)
        {
            name = rhs.name;
            level = rhs.level;
            for(auto& op: rhs.outputters)
                outputters.push_back(op);
        }
        return *this;
    }

    bool operator==(const ConfLogItem& rhs) const
    {
        return name == rhs.name && level == rhs.level && outputters == rhs.outputters;
    }

public:
    class Outputter
    {
    
    public:
        Outputter(){}
        Outputter(const Outputter& op)
        {
            type = op.type;
            file = op.file;
        }

        Outputter& operator=(const Outputter& rhs)
        {
            if(this != &rhs)
            {
                type = rhs.type;
                file = rhs.file;
            }

            return *this;
        } 

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
    std::vector<Outputter> outputters;
};

template<>
class ItemConverter<ConfLogItem, std::string>
{
public:
    std::string operator()(const ConfLogItem& log)
    {
        YAML::Node node;
        node["name"] = log.name;
        node["level"] = log.level;

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
class ItemConverter<std::string, ConfLogItem>
{
public:
    ConfLogItem operator()(const std::string& str)
    {
        ConfLogItem item;
        YAML::Node node = YAML::Load(str);
        if(!node.IsMap())
            return item;

        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) 
        {
            std::string key = it->first.as<std::string>();
            
            if(key == "name") item.name = it->second.as<std::string>();
            if(key == "level")  item.level = it->second.as<std::string>();
            if(key == "outputters")
            {
                if(!it->second.IsSequence())    continue;
                for(int i = 0; i < it->second.size(); ++i)
                {
                    if(!it->second[i].IsMap())  continue;

                    ConfLogItem::Outputter outputter;
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

void visit()
{
    Config::Visit([](ConfigItemBase::ptr cfg)
        {
            std::string name = cfg->getName();
            if(name == "logs")
            {
                ConfigItem<std::vector<ConfLogItem>>::ptr log = std::dynamic_pointer_cast<ConfigItem<std::vector<ConfLogItem>>>(cfg);
                auto logConf = log->getValue();
                for(auto& logItem: logConf)
                {
                    LOG_INFO(LOG_NAME("system")) << "log.name: " << logItem.name;
                    LOG_INFO(LOG_NAME("system")) << "log.level: " << logItem.level;
                    for(auto& op: logItem.outputters)
                    {
                        if(!op.type.empty()) LOG_INFO(LOG_NAME("system")) << "log.outputter.type: " << op.type;
                        if(!op.file.empty()) LOG_INFO(LOG_NAME("system")) << "log.outputter.file: " << op.file;  
                    }
                }
            }

            if(name.find("servers") != std::string::npos)
            {
                LOG_INFO(LOG_NAME("system")) << cfg->getName() << ", " << cfg->getDescription() << ", " << cfg->toString();
            }

        }
    );
}

int main(int argc, char* argv[])
{
    FileSystem::WorkPath("/home/silen/projs/uiconnect/uiconnect/server/bin/");
    
    LOG_INFO(LOG_NAME("system")) << "add expected config item and default value";

    // add expected config item, type and default value
    Config::SearchOrAdd("logs", std::vector<ConfLogItem>(), "");

    // add expected config item, type and default value
    Config::SearchOrAdd("servers.address", std::vector<std::string>(), "tcp address");
    Config::SearchOrAdd("servers.keepalive", int(20), "keepalive timespan");
    Config::SearchOrAdd("servers.timeout", int(300), "timeout set");
    Config::SearchOrAdd("servers.name", std::string("silen"), "server name");
    Config::SearchOrAdd("servers.accept_worker", std::string("no"), "accpet worker");
    Config::SearchOrAdd("servers.io_worker", std::string("no"), "io worker");
    Config::SearchOrAdd("servers.process_worker", std::string("no"), "process work");
    Config::SearchOrAdd("servers.type", std::string("tcp"), "type");

    // visit default config
    visit();

    LOG_INFO(LOG_NAME("system")) << "load config items from config files";
    // load setting from config
    Config::LoadFromConfDir(FileSystem::WorkPath() + "/conf", false);
    // revisit
    visit();
    return 0;
}