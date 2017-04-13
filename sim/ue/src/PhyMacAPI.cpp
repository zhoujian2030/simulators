/*
 * PhyMacAPI.cpp
 *
 *  Created on: Nov 04, 2016
 *      Author: j.zhou
 */

#include "PhyMacAPI.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#include "pktQueue.h"
#endif
#include "UeScheduler.h"
#include "StsCounter.h"

using namespace ue;
#ifdef OS_LINUX
using namespace cm;
using namespace net;
#endif

#ifndef OS_LINUX
extern "C" void* InitUePhySim() {
	LOG_DBG(UE_LOGGER_NAME, "[%s], Entry\n", __func__);
	PhyMacAPI* pPhyMacAPI = new PhyMacAPI(0);
	return (void*)pPhyMacAPI;
}

extern "C" void HandleSubFrameInd(void* phyUeSim, UInt16 sfnsf) {
	((PhyMacAPI*)phyUeSim)->handleSubFrameInd(sfnsf);
}

extern "C" void HandleDlDataRequest(void* phyUeSim, UInt8* buffer, SInt32 length) {
	((PhyMacAPI*)phyUeSim)->handleDlDataRequest(buffer, length);
}
#endif

// ----------------------------------------
PhyMacAPI::PhyMacAPI(UInt8* theBuffer)
{
	LOG_TRACE(UE_LOGGER_NAME, "[%s], Entry\n", __func__);
	m_recvBuff = theBuffer;
	m_globalTick = 0;
	m_isRunning = TRUE;
	m_stsCounter = StsCounter::getInstance();

    m_ueScheduler = new UeScheduler(this, m_stsCounter);

    memset(m_rachBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_schBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_crcBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_harqBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_srBuffer, 0, SOCKET_BUFFER_LENGTH);
    memset(m_ueConfigMsgBuffer, 0, SOCKET_BUFFER_LENGTH);

#ifdef OS_LINUX
    m_l2Address.port = L2_SERVER_PORT;
    Socket::getSockaddrByIpAndPort(&m_l2Address.addr, L2_SERVER_IP, L2_SERVER_PORT);

    m_txL2Socket = new UdpSocket();
#endif
}

// ----------------------------------------
PhyMacAPI::~PhyMacAPI() {

}

#ifdef OS_LINUX
// ----------------------------------------
void PhyMacAPI::handleRecvResult(UdpSocket* theSocket, int numOfBytesRecved) {
    LOG_DBG(UE_LOGGER_NAME, "[%s], numOfBytesRecved = %d\n", __func__, numOfBytesRecved);
    logBuff(m_recvBuff, numOfBytesRecved);

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
void PhyMacAPI::handleSendResult(UdpSocket* theSocket, int numOfBytesSent) {
    LOG_DBG(UE_LOGGER_NAME, "handleSendResult()\n");
}

// ----------------------------------------
void PhyMacAPI::handleCloseResult(UdpSocket* theSocket) {
    LOG_DBG(UE_LOGGER_NAME, "handleCloseResult()\n");
}

// ----------------------------------------
void PhyMacAPI::handleErrorResult(UdpSocket* theSocket) {
    LOG_DBG(UE_LOGGER_NAME, "handleErrorResult()\n");
}

#else
// ----------------------------------------
void PhyMacAPI::handleDlDataRequest(UInt8* theBuffer, SInt32 length) {
//	LOG_DBG(UE_LOGGER_NAME, "[%s], [%d.%d], m_isRunning = %d\n", __func__, m_sfn, m_sf, m_isRunning);
	if (m_isRunning) {
		m_ueScheduler->processDlData(theBuffer, length);
	}
}

#endif

// ----------------------------------------
void PhyMacAPI::handleSubFrameInd(UInt16 sfnsf) {

    m_sfn = ( sfnsf & 0xFFF0) >> 4;
    m_sf  = sfnsf & 0x000F;
    m_globalTick++;    

    this->resetSendBuffer();

    m_ueScheduler->updateSfnSf(m_sfn, m_sf);
    if (!m_ueScheduler->schedule()) {
    	m_isRunning = FALSE;
    	return;
    } else {
    	m_isRunning = TRUE;
    }

	if (!m_isRunning) {
		return;
	}

    LOG_TRACE(UE_LOGGER_NAME, "[%s], [%d.%d], globalTick = %d\n", __func__, m_sfn, m_sf, m_globalTick);

    // send data to L2
    this->sendData();

    m_stsCounter->show();
}

// ----------------------------------------
void PhyMacAPI::sendData() {
#ifdef OS_LINUX
    if (m_rachDataLength > 0) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], send rach indication (%d): \n", __func__, m_rachDataLength);
        logBuff(m_rachBuffer, m_rachDataLength);
        m_txL2Socket->send((const char*)m_rachBuffer, m_rachDataLength, m_l2Address);
    }

    if (m_schDataLength > 0) {
        memcpy((void*)(m_schBuffer + m_schDataLength - m_schPduLength), (void*)m_schPduBuffer, m_schPduLength);
        LOG_DBG(UE_LOGGER_NAME, "[%s], send sch indication (%d): \n", __func__, m_schDataLength);
        logBuff(m_schBuffer, m_schDataLength);
        m_txL2Socket->send((const char*)m_schBuffer, m_schDataLength, m_l2Address);
    }

    if (m_crcDataLength > 0) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], send crc indication (%d): \n", __func__, m_crcDataLength);
        logBuff(m_crcBuffer, m_crcDataLength);
        m_txL2Socket->send((const char*)m_crcBuffer, m_crcDataLength, m_l2Address);
    }

    if (m_harqDataLength > 0) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], send harq indication (%d): \n", __func__, m_harqDataLength);
        logBuff(m_harqBuffer, m_harqDataLength);
        m_txL2Socket->send((const char*)m_harqBuffer, m_harqDataLength, m_l2Address);
    }

    if (m_srDataLength > 0) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], send SR indication (%d): \n", __func__, m_srDataLength);
        logBuff(m_srBuffer, m_srDataLength);
        m_txL2Socket->send((const char*)m_srBuffer, m_srDataLength, m_l2Address);
    }
#else
    //todo PHY_UE_CONFIG_RESPONSE sent to QMSS_TX_FREE_HAND_PHY_TO_LAYER2C_REPLY
    if (m_rachDataLength > 0) {
        LOG_TRACE(UE_LOGGER_NAME, "send rach indication (%d): \n", m_rachDataLength);
//        LOG_BUFFER(m_rachBuffer, m_rachDataLength);
        qmssSendMsg(QMSS_TX_FREE_HAND_PHY_TO_LAYER2D_DATAUP, QMSS_TX_HAND_PHY_TO_OTHER, (Int8*)m_rachBuffer, m_rachDataLength);
    }

    if (m_schDataLength > 0) {
        memcpy((void*)(m_schBuffer + m_schDataLength - m_schPduLength), (void*)m_schPduBuffer, m_schPduLength);
        LOG_TRACE(UE_LOGGER_NAME, "send sch indication (%d): \n", m_schDataLength);
        qmssSendMsg(QMSS_TX_FREE_HAND_PHY_TO_LAYER2D_DATAUP, QMSS_TX_HAND_PHY_TO_OTHER, (Int8*)m_schBuffer, m_schDataLength);
    }

    if (m_crcDataLength > 0) {
    	LOG_TRACE(UE_LOGGER_NAME, "send crc indication (%d): \n", m_crcDataLength);
//        LOG_BUFFER(m_crcBuffer, m_crcDataLength);
        qmssSendMsg(QMSS_TX_FREE_HAND_PHY_TO_LAYER2D_DATAUP, QMSS_TX_HAND_PHY_TO_OTHER, (Int8*)m_crcBuffer, m_crcDataLength);
    }

    if (m_harqDataLength > 0) {
    	LOG_TRACE(UE_LOGGER_NAME, "send harq indication (%d): \n", m_harqDataLength);
//        LOG_BUFFER(m_harqBuffer, m_harqDataLength);
        qmssSendMsg(QMSS_TX_FREE_HAND_PHY_TO_LAYER2D_DATAUP, QMSS_TX_HAND_PHY_TO_OTHER, (Int8*)m_harqBuffer, m_harqDataLength);
    }

    if (m_srDataLength > 0) {
    	LOG_TRACE(UE_LOGGER_NAME, "send SR indication (%d): \n", m_srDataLength);
//        LOG_BUFFER(m_srBuffer, m_srDataLength);
        qmssSendMsg(QMSS_TX_FREE_HAND_PHY_TO_LAYER2D_DATAUP, QMSS_TX_HAND_PHY_TO_OTHER, (Int8*)m_srBuffer, m_srDataLength);
    }
#endif
}

// ------------------------------------
void PhyMacAPI::sendUeConfigResp() {
#ifdef OS_LINUX

#else
    if (m_ueConfigMsgLength > 0) {
        LOG_INFO(UE_LOGGER_NAME, "[%s], send Ue Config Response (%d): \n", __func__, m_ueConfigMsgLength);
//        LOG_BUFFER(m_ueConfigMsgBuffer, m_ueConfigMsgLength);
        qmssSendMsg(QMSS_TX_FREE_HAND_PHY_TO_LAYER2C_REPLY, QMSS_TX_HAND_PHY_TO_OTHER, (Int8*)m_ueConfigMsgBuffer, m_ueConfigMsgLength);

        m_ueConfigMsgLength = 0;
        memset(m_ueConfigMsgBuffer, 0, m_ueConfigMsgLength);
    }
#endif
}

// ------------------------------------
void PhyMacAPI::resetSendBuffer() {
    memset(m_rachBuffer, 0, m_rachDataLength);
    memset(m_schBuffer, 0, m_schDataLength);
    memset(m_crcBuffer, 0, m_crcDataLength);
    memset(m_harqBuffer, 0, m_harqDataLength);
    memset(m_srBuffer, 0, m_srDataLength);
    memset(m_ueConfigMsgBuffer, 0, m_ueConfigMsgLength);

    m_rachDataLength = 0;
    m_schDataLength = 0;
    m_crcDataLength = 0;
    m_harqDataLength = 0;
    m_srDataLength = 0;
    m_ueConfigMsgLength = 0;

    m_schPduLength = 0;
}

// ------------------------------------
void PhyMacAPI::logBuff(UInt8* theBuffer, UInt32 length) {
    LOG_BUFFER(theBuffer, length);
}
    
