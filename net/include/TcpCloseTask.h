/*
 * TcpCloseTask.h
 *
 *  Created on: July 4, 2016
 *      Author: z.j
 */

#ifndef TCP_CLOSE_TASK_H
#define TCP_CLOSE_TASK_H

#include "Task.h"
#include "TcpSocket.h"

namespace net {
    
    class TcpServerWorker;
    
    class TcpCloseTask : public cm::Task {
    public:
        TcpCloseTask(TcpServerWorker* theWorker, TcpSocket* theSocket);
        virtual ~TcpCloseTask();
        virtual int execute();  
        
    private:
        TcpSocket* m_tcpSocket;
        TcpServerWorker* m_tcpServerWorker;
    };
}

#endif
