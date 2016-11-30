/*
 * Reactor.h
 *
 *  Created on: May 10, 2016
 *      Author: z.j
 */

#ifndef REACTOR_H
#define REACTOR_H

#include "ReactorThread.h"

namespace net {

    class Reactor {
    public:
        enum {
            NUM_OF_THREADS = 10 
        };

        // if there are only 8 consecutive sockets, they are expected to 
        // registered one reactor thread first, then the next 8 ones will
        // be registered to next reactor thread
        enum {
            NUM_OF_BIT_SHIFT = 3
        };

        ~Reactor();
        static Reactor* getInstance();
        static void initialize(int numOfReactors = NUM_OF_THREADS);
        
        void registerInputHandler(Socket* theSocket, SocketEventHandler* theEventHandler);
        void removeInputHandler(Socket* theSocket);
        void removeHandlers(Socket* theSocket);
        void registerOutputHandler(Socket* theSocket, SocketEventHandler* theEventHandler);

    private:
        Reactor(int numOfReactors = NUM_OF_THREADS);

        static Reactor* m_theInstance;
        static cm::MutexLock m_lock;

        ReactorThread** m_reactorThreadArray;
        int m_numOfReactors;
        // ReactorThread m_reactorThreadArray[NUM_OF_THREADS];
    };
}

#endif
