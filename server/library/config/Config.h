#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

#include "ConfigItem.hpp"

class Config
{
public:
    typedef std::unordered_map<std::string, ConfigItemBase::ptr> ConfigItemMap;
    typedef RWMutex ConfigMutex;

    template<class T>
    static typename ConfigItem<T>::ptr SearchOrAdd(const std::string& name, const T& defaultValue, const std::string& description = "")
    {
        ConfigMutex::WriteLock lock();
        auto it = GetData().find(name);
        if(it != GetData().end())
        {
            auto tmp = std::dynamic_pointer_cast<ConfigItem<T>>(it->second);
            if(!tmp)
            {
                // TODO: LOG
                return nullptr;
            }
            return tmp;
        }

        if(IsNodeNameValid(name))
        {
            // TODO: Log
            throw std::invalid_argument(name);
        }

        ConfigItem<T>::ptr v(new ConfigItem<T>(name, default_value, description));
        GetData()[name] = v;
        return v;
    }

    template<class T>
    static typename ConfigItem<T>::ptr Search(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetData().find(name);
        if(it == GetData().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigItem<T> >(it->second);
    }

    /**
     * @brief  node name only contains alphabets in "abcdefghijklmnopqrstuvwxyz._0123456789"
    */
    static bool IsNodeNameValid(const std::string& name);
    static void LoadFromYaml(const YAML::Node& root);
    static void LoadFromConfDir(const std::string& path, bool force = false);
    static ConfigItemBase::ptr SearchBase(const std::string& name);
    static void Visit(std::function<void(ConfigItemBase::ptr)> cb);
private:

    static ConfigItemMap& GetData()
    {
        static ConfigItemMap s_data;
        return s_data;
    }

    static ConfigMutex& GetMutex()
    {
        static ConfigMutex s_mutex;
        return s_mutex;
    }
};

#endif