/*
 * Task.h
 *
 *  Created on: May 21, 2016
 *      Author: z.j
 */
 
#ifndef TASK_H
#define TASK_H
 
// Interface for all kinds of tasks that need to be done by 
// worker thread. If an application wants to do some job, it
// should implement a class which inherrits this class and 
// register this task to worker's queue
namespace cm {
    
    typedef enum {
        TRC_CONTINUE,
        TRC_END,
        TRC_EMPTY
    } TaskResultCode;
    
    class Task {
    public:  
        Task();
        virtual ~Task();
        
        // Execute the task. Derived class must implement this method.
        // @return 
        //   TRC_CONTINUE if user should continue to execute tasks
        //   TRC_END if user should stop executing task 
        virtual int execute() = 0;
        
    private:
        friend class TaskQueue;
        Task* next;
    };
}
 
#endif
