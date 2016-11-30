/*
 * SocketSetInterface.h
 *
 *  Created on: Nov 04, 2016
 *      Author: j.zhou
 */

#ifndef SOCKET_SET_INTERFACE_H
#define SOCKET_SET_INTERFACE_H

#include "Socket.h"
#include "SocketEventHandler.h"

namespace net {

    class SocketSetInterface {
    public:
        virtual void* poll(int theTimeout) = 0;

        virtual void registerInputHandler(Socket* theSocket, SocketEventHandler* theEventHandler) = 0;
        virtual void removeInputHandler(Socket* theSocket) = 0;
        virtual void registerOutputHandler(Socket* theSocket, SocketEventHandler* theEventHandler) = 0;
        virtual void removeOutputHandler(Socket* theSocket) = 0;
        virtual void removeHandlers(Socket* theSocket) = 0;

        virtual int getNumberOfSocket() const = 0;
    };

}

#endif
