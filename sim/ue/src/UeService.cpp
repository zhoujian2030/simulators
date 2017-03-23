/*
 * UeService.h
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifdef OS_LINUX

#include "UeService.h"
#include <iostream>
#include "CLogger.h"

using namespace ue;
using namespace net;

// ------------------------------------------------
UeService::UeService(std::string serviceName) 
: Service(serviceName)
{
    init();
}

// ------------------------------------------------
UeService::~UeService() {

}

// ------------------------------------------------
unsigned long UeService::run() {

    // SfnSfManager* pSfnSfMgr = SfnSfManager::getInstance();
    // UInt32 sfn, sf;

    // Socket::InetAddressPort remoteAddress;
    // Int32 recvLength;   

    m_ueMacAPI = new UeMacAPI(m_recvBuffer);

    m_udpServerSocket = new UdpSocket("0.0.0.0", 9999);
    m_udpServerSocket->addSocketHandlerForNonAsync(m_ueMacAPI, (char*)m_recvBuffer, SOCKET_BUFFER_LENGTH);

    m_selectSocketSet = new SelectSocketSet();
    m_selectSocketSet->registerInputHandler(m_udpServerSocket, (SocketEventHandler*)m_udpServerSocket);    

    SelectSocketSet::SelectSocket* readySockets;
    
    while(1) {    
        readySockets = ( SelectSocketSet::SelectSocket*)m_selectSocketSet->poll(-1);
        // only the first ready socket is valid
        readySockets->eventHandler->handleInput(readySockets->socket);

        // m_msEvent.wait();
        // pSfnSfMgr->getSFAndSFN(&sfn, &sf);
        // LOG_DBG(UE_LOGGER_NAME, "[%s], sfn = %d, sf = %d\n",  __func__, sfn, sf);
        // while(1) {
        //     recvLength = m_udpServerSocket->receive((Int8*)m_recvBuffer, SOCKET_BUFFER_LENGTH, remoteAddress);
        //     if ( recvLength <= 0) {
        //         //LOG_DBG(UE_LOGGER_NAME, "no data receive\n");
        //         break;
        //     }
        //     LOG_DBG(UE_LOGGER_NAME, "[%s], recv data (%d): \n",  __func__, recvLength);
        //     int n = 0;
        //     for (int i=0; i<recvLength; i++) {
        //         printf("%02x ", (UInt8)m_recvBuffer[i]);
        //         if (++n == 10) {
        //             n = 0;
        //             printf("\n");
        //         }
        //     }
        //     printf("\n");
        // }
    }

    return 0;
}

#endif
