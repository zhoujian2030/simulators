/*
 * Lock.h
 *
 *  Created on: Feb 25, 2016
 *      Author: z.j
 */

#ifndef LOCK_H
#define LOCK_H

namespace cm {

    class Lock {
    public:
        virtual void lock() = 0;
        virtual void unlock() = 0;
    };
}


#endif
