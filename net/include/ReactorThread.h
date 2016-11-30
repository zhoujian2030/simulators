/*
 * ReactorThread.h
 *
 *  Created on: Apr 02, 2016
 *      Author: z.j
 */

#ifndef REACTOR_THREAD_H
#define REACTOR_THREAD_H

#include "Thread.h"
#include "MutexLock.h"
#include "EpollSocketSet.h"
#include "EventIndicator.h"

namespace net {
    
    class ReactorThread : public cm::Thread {
    public:
        ReactorThread();

        virtual ~ReactorThread();

        void registerInputHandler(Socket* theSocket, SocketEventHandler* theEventHandler);
        void removeInputHandler(Socket* theSocket);
        void removeHandlers(Socket* theSocket);
        void registerOutputHandler(Socket* theSocket, SocketEventHandler* theEventHandler);

    private:
        virtual unsigned long run();
        
        cm::Lock* m_lock;
        EpollSocketSet m_epollSocketSet;
        cm::EventIndicator m_socketSetChangeEvent;
    };

    // ---------------------------------------------------------
    inline void ReactorThread::registerInputHandler(Socket* theSocket, SocketEventHandler* theEventHandler) {
        m_epollSocketSet.registerInputHandler(theSocket, theEventHandler);
        m_socketSetChangeEvent.set();
    }

    // ---------------------------------------------------------
    inline void ReactorThread::removeInputHandler(Socket* theSocket) {
        m_epollSocketSet.removeInputHandler(theSocket);
    }

    // ---------------------------------------------------------
    inline void ReactorThread::removeHandlers(Socket* theSocket) {
        m_epollSocketSet.removeHandlers(theSocket);
    }

    // ---------------------------------------------------------
    inline void ReactorThread::registerOutputHandler(Socket* theSocket, SocketEventHandler* theEventHandler) {
        m_epollSocketSet.registerOutputHandler(theSocket, theEventHandler);
        m_socketSetChangeEvent.set();
    }
}

#endif