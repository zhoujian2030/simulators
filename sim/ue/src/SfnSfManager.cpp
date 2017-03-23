/*
 * SfnSfManager.cpp
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifdef OS_LINUX

#include "SfnSfManager.h"
#include "UeService.h"
#include "CLogger.h"
#include <string.h>

using namespace ue;
using namespace cm;
using namespace net;

SfnSfManager* SfnSfManager::m_instance = 0;
MutexLock SfnSfManager::m_lock;

// ----------------------------------------
SfnSfManager::SfnSfManager() 
: Thread("SfnSfThread"), m_sfn(0), m_sf(0)
{
     LOG_DBG(UE_LOGGER_NAME, "[%s], SfnSfManager initialized\n", __func__);
     m_udpClientSocket = new UdpSocket();
     m_udpClientSocket->makeNonBlocking();
     memset((void*)m_buffer, 0, SFN_SF_MSG_BUFF_SIZE);
}

// ----------------------------------------
SfnSfManager::~SfnSfManager() {
    
}

// ----------------------------------------
SfnSfManager* SfnSfManager::getInstance() {
    if (m_instance == 0) {
        m_lock.lock();
        if (m_instance == 0) {
            SfnSfManager* temp = new SfnSfManager();
            m_instance = temp;
        }
        m_lock.unlock();
    }

    return m_instance;
}

// -------------------------------------------
void SfnSfManager::registerService(UeService* theUeService) {
    m_ueService = theUeService;
}

// -------------------------------------------
unsigned long SfnSfManager::run() {

    // peer address 
    Socket::InetAddressPort l2Address;
    l2Address.port = L2_SERVER_PORT;
    Socket::getSockaddrByIpAndPort(&l2Address.addr, L2_SERVER_IP, L2_SERVER_PORT);

    Socket::InetAddressPort ueAddress;
    ueAddress.port = L2_SERVER_PORT;
    Socket::getSockaddrByIpAndPort(&ueAddress.addr, UE_SERVER_IP, UE_SERVER_PORT);

    UInt16 msgLength = 6;
    FAPI_l1ApiMsg_st* pSfnSfMsg = (FAPI_l1ApiMsg_st*)m_buffer;
    pSfnSfMsg->msgId = PHY_UL_SUBFRAME_INDICATION;
    pSfnSfMsg->lenVendorSpecific = 0;
    pSfnSfMsg->msgLen = 2;
    FAPI_subFrameIndication_st* msgBody = (FAPI_subFrameIndication_st*)&(pSfnSfMsg->msgBody[0]);
    Thread::sleep(1000);

    UInt32 globalTick = 0;

    while(1) {
        m_sf = (++m_sf) % 10;
        if (m_sf == 0) {
            m_sfn = (++m_sfn) % 1024;
        }
        globalTick++;

        LOG_DBG(UE_LOGGER_NAME, "[%s], Send sfn and sf to L2 and UE, globalTick = %d\n", __func__, globalTick);
        msgBody->sfnsf = (m_sfn << 4) | (m_sf & 0x000F);
        m_udpClientSocket->send((const char*)m_buffer, msgLength, l2Address);
        m_udpClientSocket->send((const char*)m_buffer, msgLength, ueAddress);

        // if (m_ueService != 0) {
        //     LOG_DBG(UE_LOGGER_NAME, "[%s], Send notification to UE service\n", __func__);
        //     m_ueService->postEvent();
        // }
        Thread::sleep(200);
    }

    return 0; 
}

#endif
