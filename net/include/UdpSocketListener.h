/*
 * UdpSocketListener.h
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifndef UDP_SOCKET_LISTENER_H
#define UDP_SOCKET_LISTENER_H

namespace net {
    
    class UdpSocket;
    
    // The socket lister is both the IN/OUT event listener, which means it should 
    // listen for both receiving socket data event and sending socket data event
    class UdpSocketListener {
    public:
        virtual void handleRecvResult(UdpSocket* theSocket, int numOfBytesRecved) = 0;
        virtual void handleSendResult(UdpSocket* theSocket, int numOfBytesSent) = 0;
        virtual void handleCloseResult(UdpSocket* theSocket) = 0;
        // TODO need to pass the error code/description
        virtual void handleErrorResult(UdpSocket* theSocket) = 0;
    };
}

#endif