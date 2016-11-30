/*
 * SctpSocket.h
 *
 *  Created on: Apr 19, 2016
 *      Author: z.j
 */

#ifndef SCTP_SOCKET_H
#define SCTP_SOCKET_H

#include "Socket.h"
#include <netinet/sctp.h>

namespace net {

    class SctpSocket : public Socket {
    public:
        SctpSocket(std::string localIp, short localPort, int saFamily = AF_INET);
        SctpSocket(int socket);
        virtual ~SctpSocket();

        virtual int recv(char* theBuffer, int buffSize, int& numOfBytesReceived, int flags = 0);
        virtual int send(const char* theBuffer, int numOfBytesToSend, int& numberOfBytesSent);
    };
}

#endif
