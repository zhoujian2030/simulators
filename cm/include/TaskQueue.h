/*
 * TaskQueue.h
 *
 *  Created on: May 19, 2016
 *      Author: z.j
 */

#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>

namespace cm {
    
    class Task;
    
    class TaskQueue {
    public:  
        TaskQueue();
        virtual ~TaskQueue();
        
        // execute the first task in the queue
        // @return 
        //   TRC_CONTINUE if user should continue to execute tasks
        //   TRC_END if user should stop executing task 
        //   TRC_EMPTY if no task is executed due to empty queue
        int executeTask();
        
        bool addTask(Task* theTask);
        int getLength() const;
        
    private:        
        Task* m_firstTask;   
        Task* m_lastTask; 
        int m_length;

        pthread_cond_t m_condition;
        pthread_mutex_t m_mutex;
    };
    
    // ---------------------------------
    inline int TaskQueue::getLength() const {
        return m_length;
    }
}

#endif
