/*
 * WorkerPool.h
 *
 *  Created on: May 26, 2016
 *      Author: z.j
 */

#ifndef WORKER_POOL_H
#define WORKER_POOL_H

#include <arpa/inet.h>

namespace cm {
    
    class Worker;
    
    class WorkerPool {

    private:
        friend class Worker;

        Worker* getWorker(unsigned int index);
        Worker* getWorker(const sockaddr_in& remoteAddr);
        int getNumberOfWorkers() const;
        
        WorkerPool(int numOfWorkers);
        ~WorkerPool();
        
        Worker** m_workerArray;
        int m_numOfWorkers;
    };
    
    // --------------------------------------------
    inline int WorkerPool::getNumberOfWorkers() const {
        return m_numOfWorkers;
    }
    
}

#endif
