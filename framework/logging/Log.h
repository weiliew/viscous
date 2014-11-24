/*
 * Log.h
 *
 *  Created on: 11 Jan 2014
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <fstream>
#include <cstring>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <sys/syscall.h>

#ifndef THREAD_ID
#define THREAD_ID syscall(SYS_gettid)
#endif

#define VF_LOG_DEBUG(logger, stream)                                        \
if(logger.getLevel() <= vf_common::LogDebug)                               \
    logger.getLogSink() << "DEBUG: " << __func__ << "(" << THREAD_ID << ") : " << stream << std::endl;

#define VF_LOG_INFO(logger, stream)                                         \
if(logger.getLevel() <= vf_common::LogInfo)                                \
    logger.getLogSink() << "INFO : " << __func__ << "(" << THREAD_ID << ") : " << stream << std::endl;

#define VF_LOG_WARN(logger, stream)                                         \
if(logger.getLevel() <= vf_common::LogWarn)                                \
    logger.getLogSink() << "WARN : " << __func__ << "(" << THREAD_ID << ") : " << stream << std::endl;

#define VF_LOG_ERROR(logger, stream)                                        \
if(logger.getLevel() <= vf_common::LogError)                               \
    logger.getLogSink() << "ERROR: " << __func__ << "(" << THREAD_ID << ") : " << stream << std::endl;

#define VF_LOG_FATAL(logger, stream)                                        \
if(logger.getLevel() <= vf_common::LogFatal)                               \
    logger.getLogSink() << "ERROR: " << __func__ << "(" << THREAD_ID << ") : " << stream << std::endl;


namespace vf_common
{

enum LogLevel
{
    LogDebug = 0,
    LogInfo,
    LogWarn,
    LogError,
    LogFatal
};

template<typename LogSink>
class Logger
{
public:
    Logger(LogSink& sink, LogLevel level = LogInfo)
    : _logSink(sink)
    , _logLevel (level)
    {
    }

    virtual ~Logger()
    {
    }

    LogLevel getLevel()
    {
        return _logLevel;
    }

    std::string getLevelStr()
    {
        switch(_logLevel)
        {
        case LogDebug:
            return std::string("DEBUG");
        case LogInfo:
            return std::string("INFO");
            break;
        case LogWarn:
            return std::string("WARN");
            break;
        case LogError:
            return std::string("ERROR");
            break;
        case LogFatal:
            return std::string("FATAL");
            break;
        default:
            return std::string("UNKNOWN");
            break;
        }
    }

    static std::string getLevelStr(LogLevel level)
    {
        switch(level)
        {
        case LogDebug:
            return std::string("DEBUG");
        case LogInfo:
            return std::string("INFO");
            break;
        case LogWarn:
            return std::string("WARN");
            break;
        case LogError:
            return std::string("ERROR");
            break;
        case LogFatal:
            return std::string("FATAL");
            break;
        default:
            return std::string("UNKNOWN");
            break;
        }
    }

    LogSink& getLogSink()
    {
        return _logSink;
    }

    void setLogLevel(LogLevel level)
    {
        _logLevel = level;
    }

    bool isLogDebug()
    {
        return _logLevel == LogDebug;
    }

private:
    LogLevel    _logLevel;
    LogSink&    _logSink;
};

}  // namespace vf_common


#endif /* LOG_H_ */
