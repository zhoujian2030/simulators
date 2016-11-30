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
#include <time.h>
#include <sys/time.h>

#define UE_LOGGER_NAME "UE"
#define FILENAME /*lint -save -e613 */( NULL == strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '/')+1): strrchr(__FILE__, '\\')+1)
#define FUNCNAME __FUNCTION__
#define LINE     __LINE__

#define LOG_DEBUG(moduleId, fmt, args...){\
        (void)moduleId;\
        struct timeval tv;\
        gettimeofday(&tv, 0);\
        int ms = tv.tv_usec/1000;\
        struct tm * tmVal =  localtime(&tv.tv_sec);\
        char date[32];\
        strftime(date, sizeof(date),"%Y-%m-%d %H:%M:%S",tmVal);\
        printf("[%s.%d] [DEBUG] [%3.3s] [%ld] [%s:%d] - [%s], ", date, ms, moduleId, pthread_self(), FILENAME,LINE, FUNCNAME);\
        printf(fmt,##args);}
#define LOG_INFO(moduleId,fmt,args...){\
        (void)moduleId;\
        struct timeval tv;\
        gettimeofday(&tv, 0);\
        int ms = tv.tv_usec/1000;\
        struct tm * tmVal =  localtime(&tv.tv_sec);\
        char date[32];\
        strftime(date, sizeof(date),"%Y-%m-%d %H:%M:%S",tmVal);\
        printf("[%s.%d] [INFO ] [%3.3s] [%ld] [%s:%d] - [%s], ", date, ms, moduleId, pthread_self(), FILENAME,LINE, FUNCNAME);\
        printf(fmt,##args);}
#define LOG_WARN(moduleId, fmt,args...){\
        (void)moduleId;\
        struct timeval tv;\
        gettimeofday(&tv, 0);\
        int ms = tv.tv_usec/1000;\
        struct tm * tmVal =  localtime(&tv.tv_sec);\
        char date[32];\
        strftime(date, sizeof(date),"%Y-%m-%d %H:%M:%S",tmVal);\
        printf("[%s.%d] [WARN ] [%3.3s] [%ld] [%s:%d] - [%s], ", date, ms, moduleId, pthread_self(), FILENAME,LINE, FUNCNAME);\
        printf(fmt,##args);}
#define LOG_ERROR(moduleId, fmt, args...){\
        (void)moduleId;\
        struct timeval tv;\
        gettimeofday(&tv, 0);\
        int ms = tv.tv_usec/1000;\
        struct tm * tmVal =  localtime(&tv.tv_sec);\
        char date[32];\
        strftime(date, sizeof(date),"%Y-%m-%d %H:%M:%S",tmVal);\
        printf("[%s.%d] [ERROR] [%3.3s] [%ld] [%s:%d] - [%s], ", date, ms, moduleId, pthread_self(), FILENAME,LINE, FUNCNAME);\
        printf(fmt,##args);}


#endif