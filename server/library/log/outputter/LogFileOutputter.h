#ifndef LOG_LOGFILEOUTPUTTER_H
#define LOG_LOGFILEOUTPUTTER_H

#include <string>
#include <memory>
#include <fstream>

#include "../LogLevel.h"
#include "../LogOutputter.h"

// persistence to file
class LogFileOutputter : public LogOutputter
{
	typedef Spinlock MutexType;
	
public:
    typedef std::shared_ptr<LogFileOutputter> ptr;
    LogFileOutputter(const std::string& fileName);
    void log(LogLevel::Level level, std::shared_ptr<LogItem> item) override;
    bool reopen();
private:
    std::string m_fileName;
    std::ofstream m_fileStream;
};
#endif