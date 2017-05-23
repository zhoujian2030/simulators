/*
 * TcpSocket.h
 *
 *  Created on: May 09, 2016
 *      Author: z.j
 */

#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "Socket.h"
#include "TcpSocketListener.h"
#include "Reactor.h"
#include "MutexLock.h"

namespace net {

    // This TcpSocket object can be created by TCP server as a new accepted TCP socket, 
    // or created as a TCP client socket in TCP client (need to call connect() after created)
    class TcpSocket : public Socket {
    public:
        TcpSocket(std::string remoteIp, unsigned short remotePort);
        virtual ~TcpSocket();

        // add a TcpSocketListener to the TCP socket in acync mode
        // need to change the socket to nonblocking if the listener 
        // is 0 before
        void addSocketListener(TcpSocketListener* socketListener);
        
        // @return a hash value generated according to remote ip and port
        unsigned int getHashValue() const;
        
        const Socket::InetAddressPort& getRemoteAddress() const;
        
        // @description called by worker to receive data from socket
        //  asynchronize mode - save the buffer pointer and register the socket
        //      to epoll (need to have the socket lister added before)
        //  synchronize mode - store received data in theBuffer
        // @param theBuffer - the buffer pointer to store received data
        // @param buffSize - buffer size of the buffer
        // @return number of bytes received in sync mode, 0 in async mode
        //      return -1 if error occurrs
        int receive(char* theBuffer, int buffSize);
        
        // @description called by TCP server/client to send data to peer
        //  asynchronize mode - try Socket::send(), if not sent, save the 
        //      buffer pointer and register the EPOLLOUT event
        //  synchronize mode - block until all data sent out
        // @param theBuffer - the data buffer to be sent
        // @param numOfBytesToSend - number of bytes to be sent
        // @return  number of bytes sent for both sync and async mode if success
        //          0 if no bytes sent and register EPOLLOUT event done
        //         -1 if error  occurred for both sync and async mode
        int send(const char* theBuffer, int numOfBytesToSend);
        
        // Called by TCP client to connect to a TCP server
        //  asynchronize mode:
        //      not supported yet TODO
        //  synchronize mode:
        //      block on connection to TCP server, return after connect success.
        //
        // Arguments: N/A 
        //
        // Return: 
        //  void
        //
        // Exceptions:
        //  IoException - If error occurrs on connect(). The socket is closed before
        //      throw this exception. The application should handle this exception as
        //      it may happen when network condition is bad or remote server is unreachable.
        //
        // Note:
        //  if need connect retry, need to create a new TcpSocket
        void connect();
        
        // Called by reactor or worker thread to close the TCP socket.
        void close();

    protected:
        // Only TcpServerSocket is allowed to create a TcpSocket with an
        // created(connected) socket fd.
        friend class TcpServerSocket;

        TcpSocket(int socket, Socket::InetAddressPort& theRemoteAddrPort);

        virtual void handleInput(Socket* theSocket);
        virtual void handleOutput(Socket* theSocket);

    private: 
        typedef enum {
            // for new created socket in client side
            TCP_IDLE,
            
            // for new created socket in server side, initialize state 
            // is TCP_CONNECTED
            TCP_CONNECTED,

            // client connects to server asynchronously
            TCP_CONNECTING,
            
            // register the socket to epoll for EPOLLIN event, waiting
            // for data coming
            TCP_RECEIVING,
            
            // register the socket to epoll for EPOLLOUT event, waiting
            // for data sending
            TCP_SENDING,

            // register the socket to epoll for both EPOLLIN and EPOLLOUT
            // event
            TCP_SENDING_RECEIVING,

            // close after error
            TCP_ERROR_CLOSING,
            
            TCP_CLOSING,
            TCP_CLOSED,
            TCP_ERROR
        } TcpConnectState;
        
        TcpConnectState m_tcpState;
        Reactor* m_reactor;
        std::string m_remoteIp;
        Socket::InetAddressPort m_remoteAddrAndPort;
        
        // pointer of buffer to store receving data
        char* m_recvBuffer;
        // size of the receiving pointer
        int m_recvBufferSize;
        
        // data buffer to be sent
        const char* m_sendBuffer;
        // number bytes of data to be sent
        int m_sendBuferSize;
        int m_numOfBytesSent;
        
        TcpSocketListener* m_socketListener;

        cm::Lock* m_lock;
    };
    
    // --------------------------------------------
    inline unsigned int TcpSocket::getHashValue() const {
        return m_remoteAddrAndPort.addr.sin_addr.s_addr + m_remoteAddrAndPort.port;
    }

    // --------------------------------------------
    inline const Socket::InetAddressPort& TcpSocket::getRemoteAddress() const {
        return m_remoteAddrAndPort;
    }
}

#endif
