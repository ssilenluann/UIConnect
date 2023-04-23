#include "log/LogConfItem.h"
#include "ProgramOpt.h"
#include <yaml-cpp/yaml.h>

#include <vector>
#include <map>
#include <iostream>
#include <string>

void visit()
{
    Config::Visit([](ConfigItemBase::ptr cfg)
        {
            std::string name = cfg->getName();
            if(name == "logs")
            {
                ConfigItem<std::vector<LogConfItem>>::ptr log = std::dynamic_pointer_cast<ConfigItem<std::vector<LogConfItem>>>(cfg);
                auto logConf = log->getValue();
                for(auto& logItem: logConf)
                {
                    LOG_INFO(LOG_NAME("system")) << "log.name: " << logItem.name;
                    LOG_INFO(LOG_NAME("system")) << "log.level: " << logItem.level;
                    LOG_INFO(LOG_NAME("system")) << "log.pattern: " << logItem.pattern;
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
    FileSystem::WorkPath("/home/silen/projs/uiconnect/uiconnect/server/bin");
    
    LOG_INFO(LOG_NAME("system")) << "add expected config item and default value";

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