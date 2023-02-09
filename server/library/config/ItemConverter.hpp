#ifndef CONFIG_ITEMCONVERTER_HPP
#define CONFIG_ITEMCONVERTER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

// convert item content from source type(S) to target type(T)
template<class S, class T>
class ItemConverter
{
public:
    T operator()(const S& v)
    {
        return boost::lexical_cast<T>(v);
    }
};

template<class T>
class ItemConverter<std::string, std::vector<T>>
{
public:
    std::vector<T> operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++)
        {
            ss.str("");
            ss << node[i];
            vec.push_back(ItemConverter<std::string, T>()(ss.str()));
        }

        return vec;
    }
};

template<class T>
class ItemConverter<std::vector<T>, std::string>
{
public:
    std::string operator()(const std::vector<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i: v)
        {
            node.push_back(YAML::Load(ItemConverter<T, std::string>()(i)));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class ItemConverter<std::string, std::list<T>>
{
public:
    std::list<T> operator()(std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        std::list<T> list;
        std::stringstream ss;

        for(int i = 0; i < node.size(); i++)
        {
            ss.str("");
            ss << node[i];
            list.push_back(ItemConverter<std::string, T>()(ss.str()));
        }

        return list;
    }   
};

template<class T>
class ItemConverter<std::list<T>, std::string>
{
public:
    std::string operator()(std::list<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i: v)
        {
            node.push_back(ItemConverter<T, std::string>()(i));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }

};


#endif