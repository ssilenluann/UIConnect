#ifndef PROGRAMOPT_HPP
#define PROGRAMOPT_HPP
#define BOOST_PROGRAM_OPTIONS_NO_LIB

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <boost/program_options.hpp>


#include "Mutex.h"
#include "Singleton.h"
#include "utils/FileSystem.h"

// encapsulate the program's work environment variable
class ProgramOpt
{
public:
    virtual ~ProgramOpt(){}

    bool parse(int argc, char **argv);

    // add option and description
    void addOptions(const std::string& param, const std::string& description);

    // add program option and set default value (user set this option but didn't specify the value)
    template<class T>
    void addOptionsImplicit(const std::string& param, const T& val, const std::string& description)
    {
        RWMutex::WriteLock lock(m_mutex);
        m_opts.add_options()(param.c_str(), boost::program_options::value<T>()->implicit_value(val), description.c_str());
    }

    // add program option and set default value (user didn't set this option)
    template<class T>
    void addOptionsDefault(const std::string& param, const T& val, const std::string& description)
    {
        RWMutex::WriteLock lock(m_mutex);
        m_opts.add_options()(param.c_str(), boost::program_options::value<T>()->default_value(val), description.c_str());
    }

    // add program option whose value is an array
    template<class T>
    void addOptionsMultiToken(const std::string& param, const std::string& description)
    {
        RWMutex::WriteLock lock(m_mutex);
        m_opts.add_options()(param.c_str(), boost::program_options::value<std::vector<T>>()->multitoken(), description.c_str());
    }

    // check whether user set a option
    bool hasParam(const std::string& param);

    // delete user input after parse
    void delParam(const std::string& param);

    template<class T>
    T get(const std::string &param, const T &default_value)
    {
        RWMutex::ReadLock lock(m_mutex);
        return m_args.find(param) == m_args.end() ? default_value : m_args.at(param).as<T>();
    }

    boost::program_options::variables_map& getArgs()
    {
        return m_args;
    }

    boost::program_options::options_description& getOpts()
    {
        return m_opts;
    }

    bool setEnv(const std::string& param, const std::string& val);

    std::string getEnv(const std::string& param, const std::string& default_value = "");
    std::string getAbsolutePath(const std::string& path) const;
    std::string getAbsoluteWorkPath(const std::string& path) const;
    std::string getConfigPath();

private:
    RWMutex m_mutex;

    std::string m_program;  // process name
    std::string m_exe;  // process full path
    std::string m_cwd;  // process directory

    boost::program_options::options_description m_opts;
    boost::program_options::variables_map m_args;
};

typedef Singleton<ProgramOpt> EnvMgr;
#endif