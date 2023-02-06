#ifndef LOG_LOGFORMATTER_H
#define LOG_LOGFORMATTER_H

#include <memory>
#include <vector>

#include "LogItem.h"
#include "Logger.h"

class LogFormatter
{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    /**
     * @brief construction
     * @param[in] pattern format of log content
     * @details 
     *  %m message
     *  %p log level
     *  %r elapsed time
     *  %c log name
     *  %t thread id
	 *  %F fiber id
     *  %n \n
     *  %d time
     *  %f file name
     *  %l line name
     *  %T \t
     *  %N thread name
     *
     *  default: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
    LogFormatter(const std::string& pattern);

    /**
     * @brief return formated log content
     */
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogItem::ptr item);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogItem::ptr item);
public:

    /**
     * @brief get format item
     */
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
 
        virtual ~FormatItem() {}
        /**
         * @brief format log content and put it to ostream
         */
        virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogItem::ptr item) = 0;
    };

    /**
     * @brief resolve the input format string
     */
    void init();

    /**
     * @brief whether the input format string is valid
     */
    bool isError() const { return m_error;}

    const std::string getPattern() const { return m_pattern;}
private:

    std::string m_pattern;
    // patterns after resolved
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;

};

#endif