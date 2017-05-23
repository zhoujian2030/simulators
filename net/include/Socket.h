/*
 * Socket.h
 *
 *  Created on: Apr 05, 2016
 *      Author: z.j
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

#include "SocketEventHandler.h"

namespace net {

    typedef enum {
        SKT_SUCC,
        SKT_ERR,
        SKT_WAIT
    } SocketErrCode;

    // currently only support IPV4 TCP/SCTP socket
    class Socket : protected SocketEventHandler {
    public:

        struct InetAddressPort {
            struct sockaddr_in addr;
            unsigned short port;
        };

        // Create a socket of TCP, UDP or SCTP
        //  protocol: 0 - TCP/UDP
        //            132 (IPPROTO_SCTP) - SCTP
        //
        // Exception:
        //  IoException - if fail to create socket
        Socket(
            std::string localIp, 
            unsigned short localPort, 
            int socketType = SOCK_STREAM, 
            int protocol = 0, 
            int saFamily = AF_INET);

        Socket(int socket, int socketType = SOCK_STREAM);
        virtual ~Socket();

        // Only applicable for TCP server socket to bind local
        // ip and port before listen.
        // 
        // Return: void
        // 
        // Exception:
        //  IoException - if error occurrs in bind
        void bind();

        // Listen on the socket in server side.
        //
        // Arguments:
        //  backlog - max number of connection supported 
        //            default 10000
        // 
        // Return: void
        // 
        // Exception:
        //  IoException - if error occurrs in listen
        void listen(int backlog=10000);

        // Accept a new connection from remote client. If the socket
        // is set nonblocking, it will return SKT_WAIT immediately if
        // no new connection.
        //
        // Arguments:
        //  theSocket - new socket for the new connection
        //  theRemoteAddrPort - ip and port of the remote client
        // 
        // Return:
        //  true  - if accept new connection
        //  false - if no connection accepted for nonblocking mode
        //
        // Exception:
        //  IoException - if error occurrs in accept
        bool accept(int& theSocket, InetAddressPort& theRemoteAddrPort);

        // Only applicable for TCP client to connect to remote server.
        // If the socket is set nonblocking, it will return SKT_WAIT
        // immediately if connect not complete (need 3 handshake)
        //
        // Arguments:
        //  theRemoteAddrPort - ip and port of remote server
        //
        // Return:
        //  true  - if connect success
        //  false - if not connect complete for nonblocking mode
        // 
        // Exception:
        //  IoException - if error occurrs in connect
        bool connect(const InetAddressPort& theRemoteAddrPort);

        // Close socket
        void close();

        // Receive data from socket to a pre-allocated buffer
        //
        // Arguments:
        //  theBuffer - pointer of the buffer to save the received data
        //  buffSize - size of the buffer to save the data
        //  numOfBytesReceived - actual number bytes of data received
        //  flags - default 0
        // 
        // Return:
        //  SKT_SUCC - if recv data success on the socket
        //  SKT_WAIT - no data available on the socket for nonblocking mode
        //  SKT_ERR  - error occurrs when recv
        // 
        // Exception:
        //  std::invalid_argument - if theBuffer is null pointer
        virtual int recv(char* theBuffer, int buffSize, int& numOfBytesReceived, int flags = 0);
        // Identical to recv() with flags set to 0
        int read(char* theBuffer, int buffSize, int& numOfBytesReceived);

        // Receive data from UDP socket to a pre-allocated buffer
        //
        // Arguments:
        //  theBuffer - pointer of the buffer to save the received data
        //  buffSize - size of the buffer to save the data
        //  numOfBytesReceived - actual number bytes of data received
        //  flags - default 0
        //  theRemoteAddrPort - the remote address who sends the UDP data
        // 
        // Return:
        //  SKT_SUCC - if recv data success on the socket
        //  SKT_WAIT - no data available on the socket for nonblocking mode
        //  SKT_ERR  - error occurrs when recv
        // 
        // Exception:
        //  std::invalid_argument - if theBuffer is null pointer
        virtual int recvfrom(char* theBuffer, int buffSize, int& numOfBytesReceived, InetAddressPort& theRemoteAddrPort);

        // Send data from a buffer to peer on socket
        //
        // Arguments:
        //  theBuffer - pointer of the buffer to be sent
        //  numOfBytesToSend - number of bytes to be sent
        //  numberOfBytesSent - actual number bytes of data sent
        // 
        // Return:
        //  SKT_SUCC - if send data success on the socket
        //  SKT_WAIT - no data sent on the socket for nonblocking mode
        //  SKT_ERR  - error occurrs when send
        // 
        // Exception:
        //  std::invalid_argument - if theBuffer is null pointer
        virtual int send(const char* theBuffer, int numOfBytesToSend, int& numberOfBytesSent);
        int write(const char* theBuffer, int numOfBytesToSend, int& numberOfBytesSent);

        // Send data from a buffer to peer on UDP socket
        //
        // Arguments:
        //  theBuffer - pointer of the buffer to be sent
        //  numOfBytesToSend - number of bytes to be sent
        //  numberOfBytesSent - actual number bytes of data sent
        //  theRemoteAddrPort - the peer address that data to be sent to
        // 
        // Return:
        //  SKT_SUCC - if send data success on the socket
        //  SKT_WAIT - no data sent on the socket for nonblocking mode
        //  SKT_ERR  - error occurrs when send
        // 
        // Exception:
        //  std::invalid_argument - if theBuffer is null pointer
        virtual int sendto(const char* theBuffer, int numOfBytesToSend, int& numberOfBytesSent, InetAddressPort& theRemoteAddrPort);

        void makeNonBlocking();
        void makeBlocking();

        // Set send buffer size of the socket
        //
        // Arguments:
        //  size - size of the send buffer (in byte)
        //
        // Return:
        //  void
        void setSendBufferSize(int size);

        // Get send buffer size of the socket
        //
        // Arguments:
        //  N/A
        //
        // Return:
        //  int - size (in byte) of the send buffer
        int getSendBufferSize();

        // Set receive buffer size of the socket
        //
        // Arguments:
        //  size - size of the receive buffer (in byte)
        //
        // Return:
        //  void
        void setRecvBufferSize(int size);

        // Get receive buffer size of the socket
        //
        // Arguments:
        //  N/A
        //
        // Return:
        //  int - size (in byte) of the receive buffer
        int getRecvBufferSize();

        // Check if the socket is ready to handle read/write event
        //
        // Arguments:
        //  N/A
        // 
        // Return:
        //  bool - true if ready
        bool isReady() const;

        int getSocket() const;
        
        // @description - check if it is a server socket for listening
        // @return true if it is server socket
        bool isServerSocket() const;
        
        // @description - save user data for further user
        // @param theUserData - the user data
        // @param void
        void setUserData(void* theUserData);
        
        // @description - retrieve user data saved before
        // @return void* - the user data pointer
        void* getUserData() const;

        static std::string getHostAddress(struct sockaddr* sockaddr);
        static void getSockaddrByIpAndPort(struct sockaddr_in* sockaddr, std::string ip, unsigned short port);

    private: 
        // As the SocketEventHandler is protected inherrited, only
        // the Socket's friend classes are allowed to call its 
        // handler callback function like handleInput(), etc. 
        // So we should define ReactorThread as Socket's friend class
        friend class ReactorThread;
#ifdef SCTP_SUPPORT
        friend class SctpSocket;
#endif
        friend class TcpSocket;
        friend class UdpSocket;
        
        typedef enum {
            // The socket is successfull created by socket()
            CREATED,

            // The socket is successfull binded with local ip
            // and port by bind()
            // Only valid for server socket
            BINDED,

            // The socket is listenning by listen()
            // Only valid for server socket
            LISTENING,

            // For server, accept a new connection from client, 
            // new socket is created
            // For client, the socket is successfull connected
            // to a server
            CONNECTED,

            // Initial state for a new socket, or the socket is
            // already closed
            CLOSED
        } State;
        
        enum {
            SERVER_SOCKET,
            CONNECT_SOCKET,
            CLIENT_SOCKET
        };

        // socket fd of the server or client
        int m_socket;
        
        // socket type, like SOCK_STREAM
        int m_socketType;
        
        int m_role;

        // local ip and port
        std::string m_localIp;
        unsigned short m_localPort;
        struct sockaddr_in m_localSa;

        State m_state;
        
        void* m_userData;
    };

    // ----------------------------------------------------------
    inline bool Socket::isReady() const {
        if (!isServerSocket()) {
            return (m_state == CONNECTED);
        } else {
            return (m_state != CLOSED);
        }
    }

    // ----------------------------------------------------------
    inline int Socket::getSocket() const {
        return m_socket;
    }
    
    // ----------------------------------------------------------
    inline bool Socket::isServerSocket() const {
        return (m_role == SERVER_SOCKET);
    }
    
    // ----------------------------------------------------------
    inline void Socket::setUserData(void* theUserData) {
        m_userData = theUserData;
    }
    
    // ----------------------------------------------------------
    inline void* Socket::getUserData() const {
        return m_userData;
    }
}

#endif
