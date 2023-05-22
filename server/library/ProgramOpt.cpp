#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include "config/Config.h"
#include "ProgramOpt.h"
#include "log/Logger.h"
static Logger::ptr g_logger = LOG_ROOT();

bool ProgramOpt::parse(int argc, char **argv)
{
    int pid = getpid();
    bool retp = true;
    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, m_opts), m_args);
        boost::program_options::notify(m_args);
    }
    catch(const boost::wrapexcept<boost::program_options::unknown_option>& e)
    {
        LOG_INFO(g_logger) << e.what() << '\n';
        retp = false;
    }
    catch(const boost::wrapexcept<boost::program_options::invalid_option_value>& e)
    {
        LOG_INFO(g_logger) << e.what() << '\n';
        retp = false;
    }
    catch(const boost::wrapexcept<boost::bad_any_cast>& e)
    {
        LOG_INFO(g_logger) << e.what() << '\n';
        retp = false; 
    }
    
    m_exe = FileSystem::WorkPath();
    m_cwd = FileSystem::Dirname(m_exe);
    m_program = FileSystem::Filename(m_exe);
    return retp;
}

void ProgramOpt::addOptions(const std::string &param, const std::string &description)
{    
    RWMutex::WriteLock lock(m_mutex);
    m_opts.add_options()(param.c_str(), description.c_str());
}

bool ProgramOpt::hasParam(const std::string &param)
{
    RWMutex::ReadLock lock(m_mutex);
    return m_args.count(param) > 0;
}

void ProgramOpt::delParam(const std::string &param)
{
    RWMutex::WriteLock lock(m_mutex);
    m_args.erase(param);
}

bool ProgramOpt::setEnv(const std::string &param, const std::string &val)
{
    return setenv(param.c_str(), val.c_str(), 1) >= 0;
}

std::string ProgramOpt::getEnv(const std::string &param, const std::string &default_value)
{
    const char* v = getenv(param.c_str());
    return v == nullptr ? default_value : v;
}

std::string ProgramOpt::getAbsolutePath(const std::string &path) const
{
    if(path.empty())    return "/";
    if(path[0] == '/')  return path;

    return m_cwd + path;
}

std::string ProgramOpt::getAbsoluteWorkPath(const std::string &path) const
{
    if(path.empty())    return "/";
    if(path[0] == '/')  return path;

    static ConfigItem<std::string>::ptr s_server_work_path = Config::Search<std::string>("server.work_path");
    return s_server_work_path->getValue() + "/" + path;
}

std::string ProgramOpt::getConfigPath()
{
    return getAbsolutePath(get<std::string>("c", "conf"));
}

