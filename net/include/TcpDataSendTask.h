/*
 * TcpDataSendTask.h
 *
 *  Created on: June 10, 2016
 *      Author: z.j
 */

#ifndef TCP_DATA_SEND_TASK_H
#define TCP_DATA_SEND_TASK_H

#include "Task.h"

namespace net {
    
    class TcpServerWorker;
    class TcpData;

    class TcpDataSendTask : public cm::Task {
    public:
        TcpDataSendTask(TcpServerWorker* theWorker, TcpData* theTcpData);

        virtual ~TcpDataSendTask();
        virtual int execute();

    private:
        TcpServerWorker* m_tcpServerWorker;
        TcpData* m_tcpData;
    };
}

#endif
