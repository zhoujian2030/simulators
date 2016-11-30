/*
 * SysctlUtil.h
 *
 *  Created on: June 12, 2016
 *      Author: z.j
 */

#ifndef SYSCTL_UTIL_H
#define SYSCTL_UTIL_H

namespace cm {

    class SysctlUtil {
    public:
        static unsigned int getMaxWmem();
        static unsigned int getMaxRmem();

    private:
        static unsigned int getNetCoreValue(int valueName);
    };
}

#endif
