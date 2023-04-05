#ifndef LOG_LOGFILEOUTPUTTER_H
#define LOG_LOGFILEOUTPUTTER_H

#include <string>
#include <memory>
#include <fstream>

#include "../LogLevel.h"
#include "../LogOutputter.h"
#include "../../utils/FileSystem.h"

// persistence to file
class LogFileOutputter : public LogOutputter
{
	typedef Spinlock MutexType;
	
public:
    typedef std::shared_ptr<LogFileOutputter> ptr;
    LogFileOutputter(const std::string fileName = "", std::string pattern = "");
    void log(Logger::ptr logger, LogLevel::Level level, LogItem::ptr item) override;
    std::string toYamlString() override;
    bool reopen();
    bool reopen(const std::string fileName);

private:
    std::string m_fileName;
    std::ofstream m_fileStream;
    uint64_t m_lastTime = 0; // list time open target save file
};
#endif