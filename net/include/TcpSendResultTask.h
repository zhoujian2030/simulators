/*
 * TcpSendResultTask.h
 *
 *  Created on: June 17, 2016
 *      Author: z.j
 */

#ifndef TCP_SEND_RESULT_TASK_H
#define TCP_SEND_RESULT_TASK_H

#include "Task.h"

namespace net {
    
    class TcpServerWorker;
    class TcpSocket;

    class TcpSendResultTask : public cm::Task {
    public:
        TcpSendResultTask(TcpServerWorker* theWorker, TcpSocket* theTcpSocket, int numOfBytesSent);

        virtual ~TcpSendResultTask();
        virtual int execute();

    private:
        TcpServerWorker* m_tcpServerWorker;
        TcpSocket* m_tcpSocket;
        int m_numOfBytesSent;
    };
}

#endif
