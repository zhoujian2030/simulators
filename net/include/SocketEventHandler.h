/*
 * SocketEventHandler.h
 *
 *  Created on: Apr 09, 2016
 *      Author: z.j
 */

#ifndef SOCKET_EVENT_HANDLER_H
#define SOCKET_EVENT_HANDLER_H

namespace net {

    class Socket;

    // The Reactor users like the application over TCP/UDP socket
    // have to implement this interface, for example HTTP which is
    // over TCP. The reactor thread will call these interfaces when
    // when data can be received or sent or there is exception on 
    // one socket
    class SocketEventHandler {
    public:

        // Called by reactor when data can be received on the socket
        // without blocking
        virtual void handleInput(Socket* theSocket);

        // Called by reactor when data can be sent to the socket
        // without blocking
        virtual void handleOutput(Socket* theSocket);

        // Call by reactor when an error occured on the socket
        virtual void handleException(Socket* theSocket);
    };
}

#endif
