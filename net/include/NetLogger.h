/*
 * NetLogger.h
 *
 *  Created on: Apr 02, 2016
 *      Author: z.j
 */
#ifndef _NET_LOGGER_H_
#define _NET_LOGGER_H_

#include "CPPLogger.h"
#include "MutexLock.h"

#define _NET_LOOGER_NAME_ "NET"
#define _NET_LOGGER_ log4cplus::Logger::getInstance(_NET_LOOGER_NAME_)

namespace net {

class NetLogger {
public:
    static void initConsoleLog();
    static void setLogLevel(cm::Level level);

private:
    static cm::MutexLock m_lock;
    static bool s_isInited;
};

} // end of namespace net

#endif
