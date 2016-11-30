/*
 * TcpServerSocketListener.h
 *
 *  Created on: May 06, 2016
 *      Author: z.j
 */

#ifndef TCP_SERVER_SOCKET_LISTENER_H
#define TCP_SERVER_SOCKET_LISTENER_H

namespace net {

    class TcpServerSocket;
    class TcpSocket;

    class TcpServerSocketListener {
    public:

        virtual void handleAcceptResult(TcpServerSocket* serverSocket, TcpSocket* newSocket) = 0;
        virtual void handleCloseResult(TcpServerSocket* serverSocket) = 0;
    };
}


#endif
