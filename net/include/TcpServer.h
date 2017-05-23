/*
 * TcpServer.h
 *
 *  Created on: May 12, 2016
 *      Author: z.j
 */

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "TcpServerSocketListener.h"
#include "TcpServerInterface.h"
#include <string>

namespace net {

    class TcpServerWorker;
    class TcpServerSocket;
    class TcpSocket;
    class TcpServerCallback;

    class TcpServer : public TcpServerSocketListener, public TcpServerInterface {
    public:
        TcpServer(
            TcpServerCallback* theServerCallback,
            unsigned short localPort, 
            std::string localIp = "0.0.0.0", 
            int backlog = 100);
        ~TcpServer();

        void listen();
        bool isRunning() const;

        virtual void handleAcceptResult(TcpServerSocket* serverSocket, TcpSocket* newSocket);
        virtual void handleCloseResult(TcpServerSocket* serverSocket);

        // Called by TCP server user to send data to peer
        //
        // Arguments:
        //  theTcpData - The data object to be sent, must be allocated
        //      in heap memory with "new" as it will be free with "delete"
        //      when complete
        //
        // Return:
        //  void
        //
        // Exceptions:
        //  std::invalid_argument - if pointer is 0 or connection id is invalid
        virtual void sendData(TcpData* theTcpData);

    private:
        bool m_isRunning;
        TcpServerSocket* m_tcpServerSocket;       
        // array to store number of TcpServerWorker instances
        TcpServerWorker** m_tcpServerWorkerArray;
        int m_numberOfWorkers;
        TcpServerCallback* m_tcpServerCallback;
    };

    // ----------------------------------------
    inline bool TcpServer::isRunning() const {
        return m_isRunning;
    }
}

#endif
