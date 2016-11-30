/*
 * UeMacAPI.cpp
 *
 *  Created on: Nov 04, 2016
 *      Author: j.zhou
 */

#include "UeMacAPI.h"
#include "CLogger.h"
#include "UeScheduler.h"
#include "StsCounter.h"

using namespace ue;
using namespace cm;
using namespace net;

// ----------------------------------------
UeMacAPI::UeMacAPI(UInt8* theBuffer) 
: m_recvBuff(theBuffer), m_globalTick(0)
{
    m_ueScheduler = new UeScheduler(this);

    memset(m_rachBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_schBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_crcBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_harqBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_srBuffer, 0, SOCKET_BUFFER_LENGTH);

    m_l2Address.port = L2_SERVER_PORT;
    Socket::getSockaddrByIpAndPort(&m_l2Address.addr, L2_SERVER_IP, L2_SERVER_PORT);

    m_txL2Socket = new UdpSocket();
}

// ----------------------------------------
UeMacAPI::~UeMacAPI() {

}

// ----------------------------------------
void UeMacAPI::handleRecvResult(UdpSocket* theSocket, int numOfBytesRecved) {
    LOG_DEBUG(UE_LOGGER_NAME, "numOfBytesRecved = %d\n", numOfBytesRecved);
    int n = 0;
    for (int i=0; i<numOfBytesRecved; i++) {
        printf("%02x ", m_recvBuff[i]);
        if (++n == 10) {
            n = 0;
            printf("\n"); 
        }
    }
    printf("\n");

    UInt16 msgId = 0;
    UInt16 msgLen = 0;

    FAPI_l1ApiMsg_st *pL1Api = (FAPI_l1ApiMsg_st *)m_recvBuff;
    msgId = pL1Api->msgId;
    msgLen = pL1Api->msgLen;

    if (msgId == PHY_UL_SUBFRAME_INDICATION) {
        FAPI_subFrameIndication_st  *pSubFrameInd = (FAPI_subFrameIndication_st *)&pL1Api->msgBody[0];
        handleSubFrameInd(pSubFrameInd->sfnsf);   
    } else {
        m_ueScheduler->processDlData(m_recvBuff, numOfBytesRecved);
    }

}

// ----------------------------------------
void UeMacAPI::handleSubFrameInd(UInt16 sfnsf) {
    m_sfn = ( sfnsf & 0xFFF0) >> 4;
    m_sf  = sfnsf & 0x000F;
    m_globalTick++;    
    LOG_DEBUG(UE_LOGGER_NAME, "[%d.%d], globalTick = %d\n", m_sfn, m_sf, m_globalTick);

    this->resetSendBuffer();

    m_ueScheduler->updateSfnSf(m_sfn, m_sf);
    m_ueScheduler->schedule();

    // send data to L2
    this->sendData();

    StsCounter::getInstance()->show();
}

// ----------------------------------------
void UeMacAPI::sendData() {
    if (m_rachDataLength > 0) {
        LOG_DEBUG(UE_LOGGER_NAME, "send rach indication (%d): \n", m_rachDataLength);
        logBuff(m_rachBuffer, m_rachDataLength);
        m_txL2Socket->send((const char*)m_rachBuffer, m_rachDataLength, m_l2Address);
    }

    if (m_schDataLength > 0) {
        memcpy((void*)(m_schBuffer + m_schDataLength - m_schPduLength), (void*)m_schPduBuffer, m_schPduLength);
        LOG_DEBUG(UE_LOGGER_NAME, "send sch indication (%d): \n", m_schDataLength);
        logBuff(m_schBuffer, m_schDataLength);
        m_txL2Socket->send((const char*)m_schBuffer, m_schDataLength, m_l2Address);
    }

    if (m_crcDataLength > 0) {
        LOG_DEBUG(UE_LOGGER_NAME, "send crc indication (%d): \n", m_crcDataLength);
        logBuff(m_crcBuffer, m_crcDataLength);
        m_txL2Socket->send((const char*)m_crcBuffer, m_crcDataLength, m_l2Address);
    }

    if (m_harqDataLength > 0) {
        LOG_DEBUG(UE_LOGGER_NAME, "send harq indication (%d): \n", m_harqDataLength);
        logBuff(m_harqBuffer, m_harqDataLength);
        m_txL2Socket->send((const char*)m_harqBuffer, m_harqDataLength, m_l2Address);
    }

    if (m_srDataLength > 0) {
        LOG_DEBUG(UE_LOGGER_NAME, "send SR indication (%d): \n", m_srDataLength);
        logBuff(m_srBuffer, m_srDataLength);
        m_txL2Socket->send((const char*)m_srBuffer, m_srDataLength, m_l2Address);
    }
}

// ----------------------------------------
void UeMacAPI::handleSendResult(UdpSocket* theSocket, int numOfBytesSent) {
    LOG_DEBUG(UE_LOGGER_NAME, "handleSendResult()\n");
}

// ----------------------------------------
void UeMacAPI::handleCloseResult(UdpSocket* theSocket) {
    LOG_DEBUG(UE_LOGGER_NAME, "handleCloseResult()\n");
}

// ----------------------------------------
void UeMacAPI::handleErrorResult(UdpSocket* theSocket) {
    LOG_DEBUG(UE_LOGGER_NAME, "handleErrorResult()\n");
}



// ------------------------------------
void UeMacAPI::resetSendBuffer() {
    memset(m_rachBuffer, 0, m_rachDataLength);
    memset(m_schBuffer, 0, m_schDataLength);
    memset(m_crcBuffer, 0, m_crcDataLength);
    memset(m_harqBuffer, 0, m_harqDataLength);
    memset(m_srBuffer, 0, m_srDataLength);

    m_rachDataLength = 0;
    m_schDataLength = 0;
    m_crcDataLength = 0;
    m_harqDataLength = 0;
    m_srDataLength = 0;

    m_schPduLength = 0;
}

// ------------------------------------
void UeMacAPI::logBuff(UInt8* theBuffer, UInt32 length) {
    for (UInt32 i=0; i<length; i++) {
        printf("%02x ", theBuffer[i]);
        if (i%10 == 9) {
            printf("\n");
        }
    }
    printf("\n");
}
    