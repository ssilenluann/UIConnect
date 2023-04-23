#ifndef CONFIG_CONFIGITEM_HPP
#define CONFIG_CONFIGITEM_HPP

#include <memory>
#include <string>
#include <algorithm>
#include <cxxabi.h>

#include "ItemConverter.hpp"
#include "../Mutex.h"
#include "../log/Logger.h"

class ConfigItemBase
{
public:
    typedef std::shared_ptr<ConfigItemBase> ptr;

    ConfigItemBase(const std::string& name, const std::string& descriptrion = "")
    :m_name(name), m_description(descriptrion)
    {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);    
    }

    virtual ~ConfigItemBase() {}

    std::string getName() const { return m_name;}
    std::string getDescription() const { return m_description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string&) = 0;
    virtual std::string getTypeName() = 0;

protected:
    std::string m_name; // configer name
    std::string m_description; // configer description
};

template<class T, class FromStr = ItemConverter<std::string, T>, class ToStr = ItemConverter<T, std::string>>
class ConfigItem: public ConfigItemBase
{
public:
    typedef RWMutex ConfigMutex;
    typedef std::shared_ptr<ConfigItem> ptr;
    typedef std::function<void (const T& oldValue, const T& newValue)> onConfigChangedCB;

    ConfigItem(const std::string& name, const T& defaultValue, const std::string& description = "")
    : ConfigItemBase(name, description), m_val(defaultValue){}
    std::string toString() override
    {
        try
        {
            ConfigMutex::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        }
        catch(const std::exception& e)
        {
            LOG_INFO(LOG_NAME("system")) << e.what();
        }
        return "";
        
    }    

    bool fromString(const std::string& val) override
    {
        try
        {
            setValue(FromStr()(val));
        }
        catch(const std::exception& e)
        {
            LOG_INFO(LOG_NAME("system")) << e.what();
            LOG_INFO(LOG_NAME("system")) << "val: " << val;
        }
        
    }

    void setValue(const T& v)
    {
        {
            ConfigMutex::ReadLock lock(m_mutex);
            if(v == m_val)   return;

            // callback
            for(auto& i: m_cbs)
                i.second(m_val, v);
        }

        ConfigMutex::WriteLock lock(m_mutex);
        m_val = v;
    }

    T getValue()
    {
        ConfigMutex::ReadLock lock(m_mutex);
        return m_val;
    }

    std::string getTypeName() override
    {
        return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    }    

    uint64_t addListener(onConfigChangedCB cb)
    {
        static uint64_t s_cbId = 0;
        ConfigMutex::WriteLock lock(m_mutex);
        s_cbId ++;
        m_cbs[s_cbId] = cb;
        return s_cbId;
    }

    void delListener(uint64_t cbId)
    {
        ConfigMutex::WriteLock lock(m_mutex);
        m_cbs.erase(cbId);
    }

    void getListener(uint64_t cbId)
    {
        ConfigMutex::ReadLock lock(m_mutex);
        auto it = m_cbs.find(cbId);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener()
    {
        ConfigMutex::WriteLock(m_mutex);
        m_cbs.clear();
    }

private:
    ConfigMutex m_mutex;
    T m_val;
    std::map<uint64_t, onConfigChangedCB> m_cbs;
};
#endif