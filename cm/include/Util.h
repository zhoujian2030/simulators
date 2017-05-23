/*
 * Util.h
 *
 *  Created on: Jan 11, 2016
 *      Author: z.j
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>
#include <signal.h>

class Util {
public:
    static int s2i(std::string theString);

    // signal.h typedef void (*sighandler_t)(int);
    static int installSignalHandler(int signo, sighandler_t handler);
};

#endif
