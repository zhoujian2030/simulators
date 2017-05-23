/*
 * CPPLogger.h
 *
 *  Created on: Dec 01, 2015
 *      Author: z.j
 */
#ifndef _CPP_LOGGER_H_
#define _CPP_LOGGER_H_
#include <log4cplus/logger.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/loggingmacros.h>

namespace cm {

    typedef enum {
        TRACE = log4cplus::TRACE_LOG_LEVEL,
        DEBUG = log4cplus::DEBUG_LOG_LEVEL,
        INFO  = log4cplus::INFO_LOG_LEVEL,
        WARN  = log4cplus::WARN_LOG_LEVEL,
        ERROR = log4cplus::ERROR_LOG_LEVEL,
        FATAL = log4cplus::FATAL_LOG_LEVEL
    } Level;

    class CPPLogger {
    public:
        static log4cplus::Logger getLogger(const log4cplus::tstring& loggerName);
        static void initConsoleLog(const log4cplus::tstring& loggerName);
        static void initConsoleLog(log4cplus::Logger& logger);
        static void setLogLevel(const log4cplus::tstring& loggerName, log4cplus::LogLevel level);
        static void setLogLevel(log4cplus::Logger& logger, log4cplus::LogLevel level);
    };

} // namespace cm

#endif
