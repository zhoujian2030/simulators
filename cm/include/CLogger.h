/*
 * CLogger.h
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */
#ifndef C_LOGGER_H
#define C_LOGGER_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define UE_LOGGER_NAME "UE"
#define FILENAME /*lint -save -e613 */( NULL == strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '/')+1): strrchr(__FILE__, '\\')+1)
#define FUNCNAME __FUNCTION__
#define LINE     __LINE__

#define LOG_DEBUG(moduleId, fmt, args...){\
        (void)moduleId;\
        printf("[yyyy-mm-dd hh:mm:ss.xxx] [DEBUG] [%s] [%u] [%s:%d] - [%s], ",moduleId, (unsigned int)(pthread_self()), FILENAME,LINE, FUNCNAME);\
        printf(fmt,##args);}
#define LOG_INFO(moduleId,fmt,args...){\
        (void)moduleId;\
        printf("[yyyy-mm-dd hh:mm:ss.xxx] [INFO ] [%s] [%u] [%s:%d] - [%s], ",moduleId, (unsigned int)(pthread_self()), FILENAME,LINE, FUNCNAME);\
        printf(fmt,##args);}
#define LOG_WARN(moduleId, fmt,args...){\
        (void)moduleId;\
        printf("[yyyy-mm-dd hh:mm:ss.xxx] [WARN ] [%s] [%u] [%s:%d] - [%s], ",moduleId, (unsigned int)(pthread_self()), FILENAME,LINE, FUNCNAME);\
        printf(fmt,##args);}
#define LOG_ERROR(moduleId, fmt, args...){\
        (void)moduleId;\
        printf("[yyyy-mm-dd hh:mm:ss.xxx] [ERROR] [%s] [%u] [%s:%d] - [%s], ",moduleId, (unsigned int)(pthread_self()), FILENAME,LINE, FUNCNAME);\
        printf(fmt,##args);}


#endif