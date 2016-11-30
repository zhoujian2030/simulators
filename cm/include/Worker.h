/*
 * Worker.h
 *
 *  Created on: May 15, 2016
 *      Author: z.j
 */

#ifndef WORKER_H
#define WORKER_H

#include "Thread.h"
#include "WorkerPool.h"
#include "MutexLock.h"
#include "Task.h"
#include "TaskQueue.h"

#include <arpa/inet.h>

namespace cm {

    class Worker : public Thread {
    public:
        // @description - get a worker instance by index
        // @param index - the id pointing to a worker
        // @return worker instance from WorkerPool by index
        static Worker* getInstance(unsigned int index);
        static Worker* getInstance(const sockaddr_in& remoteAddr);
        
        // @description - by default create NUM_OF_WORKER_THREAD worker threads
        static void initialize(int numOfWorkers = NUM_OF_WORKER_THREAD);
        
        static int getNumberOfWorkers();
        
        bool addTask(Task* theTask);
        int getIndex() const;
        
    private:
        friend class WorkerPool;

        enum {
            NUM_OF_WORKER_THREAD = 15
        };
        
        Worker(int index);
        virtual ~Worker();
        
        virtual unsigned long run();
        
        // index for the worker thread, range 0 ... NUM_OF_WORKER_THREAD-1
        int m_index;
        TaskQueue m_taskQueue;

        static WorkerPool* m_workerPoolInstance;
        static MutexLock m_lock;
    };
    
    // --------------------------------------------
    inline int Worker::getNumberOfWorkers() {
        Worker::initialize();
        return m_workerPoolInstance->getNumberOfWorkers();
    }
    
    // ----------------------------------------
    // Add a new task into the worker's queue and send
    // out indication to notify the worker thread.
    inline bool Worker::addTask(Task* theTask) {
        bool result = m_taskQueue.addTask(theTask);
        return result;
    }
          
    // ----------------------------------------
    inline int Worker::getIndex() const {
        return m_index;
    }      
}

#endif
