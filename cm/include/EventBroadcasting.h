/*
 * EventBroadcasting.h
 *
 *  Created on: Apr 19, 2016
 *      Author: z.j
 */

#ifndef EVENT_BROADCASTING_H
#define EVENT_BROADCASTING_H

#include <pthread.h>

namespace cm {

    // Thread A calls wait() and blocks if m_eventIsSet is never set "true" before
    // Thread B calls notifyAll() to set m_eventSet and send notification to Thread A
    class EventBroadcasting {
    public:
        EventBroadcasting();
        virtual ~EventBroadcasting();

        // subscribe an event, only support maximum 32 events
        //
        // Arguments:
        //  N/A
        // 
        // Return:
        //  int : the event id that need to be passed to wait
        int subscribe();

        // wait and block on an event
        //
        // Arguments:
        //  eventId : the event id from subscribe()
        // 
        // Return:
        //  void
        void wait(int eventId);

        // broadcast event to invoke all threads
        void notifyAll();

        // reset the event 
        void reset();

    private: 
        int m_subscriberCnt;
        unsigned int m_eventSet;
        pthread_cond_t m_condition;
        pthread_mutex_t m_mutex;
        
    };
}

#endif
