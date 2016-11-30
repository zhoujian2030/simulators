/*
 * TcpServerWorker.h
 *
 *  Created on: May 25, 2016
 *      Author: z.j
 */
 
#ifndef TCP_SERVER_WORKER_H
#define TCP_SERVER_WORKER_H

#include "TcpSocketListener.h"
#include "Worker.h" 
#include "TcpConnection.h"
#include "TcpData.h"
#include <map>

namespace net {
        
    class TcpSocket;
    class TcpServerCallback;
     
    // A TcpServerWorker is only responsible and used for one Worker thread  
    // to handle new TCP connection. So the application should create the 
    // same number of TcpServerWorker instance as Worker instance. And it
    // is also the TcpSocket listener which implements the listener interfaces
    // that called by reactor thread when there is data received on the socket
    class TcpServerWorker : public TcpSocketListener {
    public:
        TcpServerWorker(cm::Worker* theWorker, TcpServerCallback* theServerCallback);
        virtual ~TcpServerWorker();
           
        // @Description - called by TcpAcceptTask to handle new accepted connection
        //      will always called by the same worker thread. It is only for the async
        //      mode, as it registers this TcpServerWorker as the socket listener and
        //      then call receive() to register this socket to reactor's epoll
        // @param theNewSocket - created by reactor for the new accepted connection
        virtual void onConnectionCreated(TcpSocket* theNewSocket);  

        // Called by TcpDataReceivedTask to handle received socket data. It is always
        // called by the same worker thread for async mode.
        //
        // Arguments:
        //  theSocket - the tcp socket that has data received
        //  numOfBytesRecved - number of bytes received
        //
        // Return:
        //  void
        virtual void onDataReceived(TcpSocket* theSocket, int numOfBytesRecved);

        // Called by TcpSendResultTask to notify send data to socket result
        //
        // Arguments:
        //  theSocket - the tcp socket that sent the data
        //  numOfBytesSent - number of bytes received
        //
        // Return:
        //  void        
        virtual void onSendResult(TcpSocket* theSocket, int numOfBytesSent);

        // Called by TcpCloseTask to handle socket close
        //
        // Arguments:
        //  theSocket - the tcp socket that is closed
        //
        // Return:
        //  void
        virtual void onConnectionClosed(TcpSocket* theSocket);
        
        // Called by TcpSocket to handle new data received on the socket. Could
        // be called by different reactor thread for different socket, need to 
        // be carefull of thread conflict
        // 
        // Arguments:
        //  theSocket - the tcp socket that has data received
        //	numOfBytesRecved - number of bytes received
        //
        // Return: 
        //  void
        virtual void handleRecvResult(TcpSocket* theSocket, int numOfBytesRecved);

        // Called by TcpSocket to notify tcp data sent result.
        // 
        // Arguments:
        //  theSocket - the tcp socket that has data received
        //	numOfBytesSent - number of bytes sent
        //
        // Return: 
        //  void     
        virtual void handleSendResult(TcpSocket* theSocket, int numOfBytesSent);

        // Called by TcpSocket to handle socket close
        //
        // Arguments:
        //  theSocket - the tcp socket that is closed
        // 
        // Return:
        //  void
        virtual void handleCloseResult(TcpSocket* theSocket);

        // Called by TcpSocket to handle socket error
        //
        // Arguments:
        //  theSocket - the tcp socket that has error occurred
        // 
        // Return:
        //  void        
        virtual void handleErrorResult(TcpSocket* theSocket);
        
        // Send response data to client
        virtual void sendData(TcpData* theTcpData);

    protected:
        friend class TcpConnection;
        
        void createConnection(TcpSocket* theNewSocket);
        
        // @get server worker id 
        // @return the worker index as the server worker id
        int getWorkerId() const;
        
        cm::Worker* m_worker;
        unsigned int m_connectionIdCounter;
        std::map<unsigned int, TcpConnection*> m_connMap;
        TcpServerCallback* m_tcpServerCallback;

    };
    
    // --------------------------------------------------
    inline int TcpServerWorker::getWorkerId() const {
        return m_worker->getIndex();
    }
     
}
 
 #endif