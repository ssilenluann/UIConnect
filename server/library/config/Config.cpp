#ifndef CONFIG_CONFIG_CPP
#define CONFIG_CONFIG_CPP

#include <iostream>

#include "Config.h"
#include "../ProgramOpt.h"
#include "../utils/FileSystem.h"
#include "../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

/**
 * @brief traverse all nodes in dfs, and save the node with this name as "parent node name"."node name"
*/
static void VisitNodesDFS(const std::string& prefix, const YAML::Node& node, std::list<std::pair<std::string, const YAML::Node>>& list)
{
    if(!Config::IsNodeNameValid(prefix))
    {
        LOG_ERROR(g_logger) << "invalid config node name: " << prefix << " : " << node;
        return;
    }

    list.emplace_back(prefix, node);
    if(node.IsMap())
    {
        for(auto& n: node)
        {
            VisitNodesDFS(prefix.empty() ? n.first.Scalar() : prefix + "." + n.first.Scalar(), n.second, list);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& root)
{
    std::list<std::pair<std::string, const YAML::Node>> nodes;
    VisitNodesDFS("", root, nodes);

    for(auto& i: nodes)
    {
        std::string key = i.first;
        if(key.empty()) continue;

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigItemBase::ptr item = SearchBase(key);
        if(item != nullptr && !i.second.Scalar().empty())
        {
            item->fromString(i.second.Scalar());
        }
        if(item != nullptr && i.second.Scalar().empty())
        {
            std::stringstream ss;
            ss << i.second;
            item->fromString(ss.str());
        }
    }
}

void Config::LoadFromConfDir(const std::string& path, bool force)
{
    std::string confDir = EnvMgr::GetInstance()->getAbsolutePath(path);
    std::list<std::string> files;
    FileSystem::GetAllFile(confDir, "yaml", files);

    for(auto& i: files)
    {
        try
        {
            YAML::Node root = YAML::LoadFile(i);
            LoadFromYaml(root);
            LOG_INFO(g_logger) << "LoadConfFile file="
                << i << " ok";
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(g_logger) << "LoadConfFile file="
                << i << " failed";
        }
        
    }
}

ConfigItemBase::ptr Config::SearchBase(const std::string& name)
{
    ConfigMutex::ReadLock lock(GetMutex());
    auto it = GetData().find(name);
    return it == GetData().end() ? nullptr : it->second;
}

void Config::Visit(std::function<void(ConfigItemBase::ptr)> cb)
{
    ConfigMutex::ReadLock lock(GetMutex());

    for(auto it = GetData().begin(); it != GetData().end(); it++)
        cb(it->second);
}

bool Config::IsNodeNameValid(const std::string& name)
{
    return name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") == std::string::npos;
}
#endif