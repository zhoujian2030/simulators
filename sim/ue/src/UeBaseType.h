/*
 * UeBaseType.h
 *
 *  Created on: Nov 08, 2016
 *      Author: j.zhou
 */

#ifndef UE_BASE_TYPE_H
#define UE_BASE_TYPE_H

#define FAPI_API

#define SOCKET_BUFFER_LENGTH 4096

typedef signed int     Int;
typedef unsigned char   UInt8;
typedef unsigned short  UInt16;
typedef unsigned int    UInt32;
typedef unsigned long    ULong32;
typedef unsigned long long   UInt64;
typedef unsigned char   UChar8;
typedef signed char   	SInt8;
typedef char            Char8;
typedef signed short    SInt16;
typedef signed int      SInt32;
typedef signed long long     SInt64;
typedef float           UDouble32;
typedef double          UDouble64;
#ifndef PNULL
#define PNULL NULL
#endif
typedef unsigned char BOOL;
#define TRUE 1
#define FALSE 0

#endif