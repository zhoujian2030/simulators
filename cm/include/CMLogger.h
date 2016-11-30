/*
 * CMLogger.h
 *
 *  Created on: Apr 02, 2016
 *      Author: z.j
 */
#ifndef CM_LOGGER_H
#define CM_LOGGER_H

#include "CPPLogger.h"

#define _CM_LOOGER_NAME_ "COM"
#define _CM_LOGGER_ log4cplus::Logger::getInstance(_CM_LOOGER_NAME_)

namespace cm {

    class CMLogger {
    public:
        static void initConsoleLog();
        static void setLogLevel(Level level);

    private:
        static bool s_isInited;
    };
}

#endif
