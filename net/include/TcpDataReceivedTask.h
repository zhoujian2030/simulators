/*
 * TcpDataReceivedTask.h
 *
 *  Created on: June 08, 2016
 *      Author: z.j
 */

#ifndef TCP_DATA_RECEIVED_TASK
#define TCP_DATA_RECEIVED_TASK

#include "Task.h"
#include "TcpSocket.h"

namespace net {
    
    class TcpServerWorker;
    
    // create by TcpServerWorker reactor thread to let worker thread 
    // handle new data received on the socket
    class TcpDataReceivedTask : public cm::Task {
    public:
        TcpDataReceivedTask(
            TcpSocket* theSocket, 
            TcpServerWorker* theWorker, 
            int numOfBytesReceived);

        virtual ~TcpDataReceivedTask();
        virtual int execute();  
        
    private:
        TcpSocket* m_tcpSocket;
        TcpServerWorker* m_tcpServerWorker;
        int m_numOfBytesReceived;
    };
}

#endif
