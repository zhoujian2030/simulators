/*
 * TcpConnection.h
 *
 *  Created on: May 31, 2016
 *      Author: z.j
 */
 
#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "DataBuffer.h"
 
namespace net {
    
    class TcpSocket;
    class TcpServerWorker;
    class TcpServerCallback;
    class TcpData;
    
    // m_recvBuffer is used to stored the data received on the socket,
    // it is written by reactor thread, but processed by worker thread,
    // so we must make sure reactor thread won't write data until worker
    // thread calls onDataReceive() to process the data
    class TcpConnection {
    public:
        TcpConnection(
            TcpSocket* theNewSocket,
            unsigned int connectionId,
            TcpServerWorker* theServerWorker,
            TcpServerCallback* theServerCallback);
        virtual ~TcpConnection();
        
        // @description - start to recv data from socket
        //      async : register the socket to epoll and return true
        //      sync : block on recving data, return false if error
        bool recvDataFromSocket();
        
        // @description - called by TcpServerWorker to handle received data
        //      in async mode. DO NOT call it in sync mode as it will block
        // @param numOfBytesRecved - number of bytes received on the socket
        void onDataReceived(int numOfBytesRecved);

        // Called by TcpServerWorker to send socket data
        void sendDataToSocket(TcpData* theTcpData);

        // Called by TcpServerWorker (worker thread) to handle the tcp socket
        // data send result.
        //
        // Arguments:
        //  numOfByteSent - the number of bytes sent
        //
        // Return:
        //  void
        void onSendResult(int numOfByteSent);
        
        // @description - called by TcpServerWorker to close connection when
        //      disconneted by peer
        void onConnectionClosed();
        
        // @description - close connection
        void close();
                
        // @description - get connection id
        // @return the unique connection id created within a TcpServerWorker
        unsigned int getConnectionId() const;
        
        // @description - get global connection id
        // @return the global unique connection id created within a TcpServer
        unsigned int getGlobalConnectionId() const;
        
    private:
        friend class TcpServerWorker;
        // @description - set connection id
        // @param connectionId - the new generated connection id
        void setConnectionId(unsigned int connectionId);
        
        typedef enum {
            READY_TO_RECV,
            RECVING
        }TcpRecvState;
        
        TcpSocket* m_tcpSocket;
        unsigned int m_connectionId;
        TcpServerWorker* m_tcpServerWorker;
        TcpServerCallback* m_tcpServerCallback;
        cm::DataBuffer* m_recvBuffer;     
        TcpRecvState m_recvState;
        
        // store the data to be sent out
        TcpData* m_sendData;
    };
    
    // -------------------------------------------------
    inline void TcpConnection::setConnectionId(unsigned int connectionId) {
        this->m_connectionId = connectionId;
    }
     
    // -------------------------------------------------
    inline unsigned int TcpConnection::getConnectionId() const {
        return m_connectionId;
    }
    

}
 
#endif
