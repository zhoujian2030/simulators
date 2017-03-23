/*
 * SfnSfManager.h
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifndef SFN_SF_MANAGER_H
#define SFN_SF_MANAGER_H

#ifdef OS_LINUX

#include "Thread.h"
#include "MutexLock.h"
#include "UdpSocket.h"
#include "FapiInterface.h"

namespace ue {

    class UeService;

    // Initialize a UE service to simulate mutiple UE 
    class SfnSfManager : public cm::Thread {
    public:        
        virtual ~SfnSfManager();

        static SfnSfManager* getInstance();
        void registerService(UeService* theUeService);

        void getSFAndSFN(UInt32* sfn, UInt32* sf);

        enum {
            SFN_SF_MSG_BUFF_SIZE = 100
        };

    private:
        SfnSfManager();
        static SfnSfManager* m_instance;
        static cm::MutexLock m_lock;

        virtual unsigned long run();

        UeService* m_ueService;

        volatile UInt32 m_sfn;
        volatile UInt32 m_sf;

        net::UdpSocket* m_udpClientSocket;
        uint8_t m_buffer[SFN_SF_MSG_BUFF_SIZE];
    };

    inline void SfnSfManager::getSFAndSFN(UInt32* sfn, UInt32* sf) {
        *sfn = m_sfn;
        *sf  = m_sf;
    }

}

#endif

#endif
