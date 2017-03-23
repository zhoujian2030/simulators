/*
 * UeService.h
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifndef UE_SERVICE_H
#define UE_SERVICE_H

#ifdef OS_LINUX

#include "Service.h"
#include "EventIndicator.h"
#include "UdpSocket.h"
#include "SfnSfManager.h"
#include "SelectSocketSet.h"
#include "UeMacAPI.h"

namespace ue {

    // Initialize a UE service to simulate mutiple UE 
    class UeService : public cm::Service {
    public:
        UeService(std::string serviceName);
        virtual ~UeService();

        void postEvent();

    private:
        virtual unsigned long run();
        
        cm::EventIndicator m_msEvent;

        net::UdpSocket* m_udpServerSocket;
        UInt8 m_recvBuffer[SOCKET_BUFFER_LENGTH];

        net::SelectSocketSet* m_selectSocketSet;
        UeMacAPI* m_ueMacAPI;
    };

    inline void UeService::postEvent() {
        m_msEvent.set();
    }

}

#endif

#endif
