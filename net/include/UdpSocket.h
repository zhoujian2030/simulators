/*
 * UdpSocket.h
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "Socket.h"
#include "UdpSocketListener.h"
#include "Reactor.h"
#include "MutexLock.h"

namespace net {

    typedef struct sockaddr_in socket_address;

    // This UdpSocket object can be created by UDP server or client
    class UdpSocket : public Socket {
    public:
        // @description create a UDP server socket
        // @param localIp - local address of the server
        // @param localPort -  the port for the server to bind to 
        UdpSocket(std::string localIp, unsigned short localPort);

        // @description create a UDP client socket
        UdpSocket();


        virtual ~UdpSocket();

        // add a UdpSocketListener to the UDP socket in acync mode
        // need to change the socket to nonblocking if the listener 
        // is 0 before
        void addSocketListener(UdpSocketListener* socketListener);
                
        // @description called by worker to receive data from socket
        //  asynchronize mode - save the buffer pointer and register the socket
        //      to epoll (need to have the socket lister added before)
        //  synchronize mode - store received data in theBuffer
        // @param theBuffer - the buffer pointer to store received data
        // @param buffSize - buffer size of the buffer
        // @return 
        //  asynchronize mode - return 0 if success, -1 if error
        //  synchronize mode - return number of bytes received (including 0 if no data 
        //      received in nonblocking mode), -1 if error
        int receive(char* theBuffer, int buffSize, InetAddressPort& theRemoteAddrPort);
        
        // @description called by UDP server/client to send data to peer
        //  asynchronize mode - try Socket::send(), if not sent, save the 
        //      buffer pointer and register the EPOLLOUT event
        //  synchronize mode - block until all data sent out
        // @param theBuffer - the data buffer to be sent
        // @param numOfBytesToSend - number of bytes to be sent
        // @return  number of bytes sent for both sync and async mode if success
        //          0 if no bytes sent and register EPOLLOUT event done
        //         -1 if error  occurred for both sync and async mode
        int send(const char* theBuffer, int numOfBytesToSend, InetAddressPort& theRemoteAddrPort);
                
        // Called by reactor or worker thread to close the UDP socket.
        void close();

        const socket_address& getLocalAddress() const;

        // async does not mean non-blocking, sync mode can also be non-blocking
        // for async mode, user should call addSocketListener() to register the
        // socket listener with Reactor initialized
        void addSocketHandlerForNonAsync(UdpSocketListener* socketListener, char* theBuffer, int bufferSize);

    protected:
        virtual void handleInput(Socket* theSocket);
        virtual void handleOutput(Socket* theSocket);

    private: 
        typedef enum {
            // for new created socket in client side
            UDP_IDLE,
            
            UDP_OPEN,

            // close after error
            UDP_ERROR_CLOSING,
            
            UDP_CLOSING,
            UDP_CLOSED,
            UDP_ERROR
        } UdpConnectState;
        
        UdpConnectState m_udpState;
        Reactor* m_reactor;
        
        // pointer of buffer to store receving data
        char* m_recvBuffer;
        // size of the receiving pointer
        int m_recvBufferSize;
        
        // data buffer to be sent
        const char* m_sendBuffer;
        // number bytes of data to be sent
        int m_sendBuferSize;
        int m_numOfBytesSent;
        
        UdpSocketListener* m_socketListener;

        cm::Lock* m_lock;
    };
    
    // --------------------------------------------
    inline const socket_address& UdpSocket::getLocalAddress() const {
        return m_localSa; 
    }
}

#endif
