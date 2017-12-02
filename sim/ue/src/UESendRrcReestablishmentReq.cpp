/*
 * UESendRrcReestablishmentReq.cpp
 *
 *  Created on: 2017-4-8
 *      Author: j.zhou
 */


#include "UESendRrcReestablishmentReq.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif
#include "PhyMacAPI.h"
#include "UeScheduler.h"
#include "StsCounter.h"

using namespace ue;

// --------------------------------------------
UESendRrcReestablishmentReq::UESendRrcReestablishmentReq(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, preamble, phyMacAPI, stsCounter)
{

}

// --------------------------------------------
UESendRrcReestablishmentReq::~UESendRrcReestablishmentReq() {

}

// --------------------------------------------
void UESendRrcReestablishmentReq::buildMsg3Data() {
    UInt32 msgLen = 0;
    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getSchBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UL_RX_ULSCH_INDICATION;

    FAPI_rxULSCHIndication_st *pULSchInd = (FAPI_rxULSCHIndication_st*)&pL1Api->msgBody[0];
    pULSchInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
    pULSchInd->numOfPdu += 1;
    // msgLen += offsetof(FAPI_rxULSCHIndication_st, ulDataPduInfo);

    FAPI_ulDataPduIndication_st *pUlDataPduInd;

    if (pULSchInd->numOfPdu == 1) {
        UInt32 schHeaderLen = offsetof(FAPI_rxULSCHIndication_st, ulDataPduInfo);
        m_phyMacAPI->addSchDataLength(FAPI_HEADER_LENGTH + schHeaderLen);
        pL1Api->msgLen += schHeaderLen;
        pUlDataPduInd = (FAPI_ulDataPduIndication_st *)&pULSchInd->ulDataPduInfo[0];
    } else {
        pUlDataPduInd = (FAPI_ulDataPduIndication_st *)&pULSchInd->ulDataPduInfo[pULSchInd->numOfPdu - 1];
    }

    pUlDataPduInd->rnti = m_rnti;
    pUlDataPduInd->length = MSG3_LENGTH;
    pUlDataPduInd->ulCqi = 2;   //TBD
    pUlDataPduInd->timingAdvance = m_ta;
    pUlDataPduInd->dataOffset = 1;  // TBD

    msgLen += sizeof(FAPI_ulDataPduIndication_st);

    // randomValue = (ueId << 32) | m_msg3.randomValue
    UInt8 msg3Buffer[MSG3_LENGTH];
    UInt32 i = 0;
    UInt16 cRnti = (m_ueId << 8) | m_ueId;  // 16 bits
    UInt16 cellId = 103;  // 9 bits
    UInt16 shortMacI = 0xa957;
    msg3Buffer[i++] = 0x20 | m_msg3.lcCCCH;
    msg3Buffer[i++] = 6;
    msg3Buffer[i++] = m_msg3.lcPadding;
    msg3Buffer[i++] = 0x00 | ((cRnti & 0xf800) >> 11);
    msg3Buffer[i++] = (cRnti & 0x07f8) >> 3;
    msg3Buffer[i++] = ((cRnti & 0x0007) << 5) | ((cellId & 0x01f0) >> 4);
    msg3Buffer[i++] = ((cellId & 0x000f) << 4) | ((shortMacI & 0xf000) >> 12);
    msg3Buffer[i++] = ((shortMacI & 0x0f00) >> 4) | ((shortMacI & 0x00f0) >> 4);
    msg3Buffer[i++] = ((shortMacI & 0x000f) << 4) | 0x04;

    msgLen += pUlDataPduInd->length;
    pL1Api->msgLen += msgLen;
    m_phyMacAPI->addSchDataLength(msgLen);

    m_phyMacAPI->addSchPduData(msg3Buffer, pUlDataPduInd->length);

    m_stsCounter->countMsg3Sent();

    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose MSG3 (RRC Connection Reestablishment Request), msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
}

// --------------------------------------------
BOOL UESendRrcReestablishmentReq::parseContentionResolutionPdu(UInt8* data, UInt32 pduLen) {
	LOG_INFO(UE_LOGGER_NAME, "[%s], %s, Contention Resolution pduLen = %d\n",  __func__, m_uniqueId, pduLen);

#ifdef OS_LINUX
    LOG_BUFFER(data, pduLen);
#endif

    BOOL result = FALSE;
    UInt8 i = 0;

    // refer to 36.321 6.1.3.4 []http://www.sharetechnote.com/html/Handbook_LTE_MAC_CE.html]
    if (pduLen != CONTENTION_RESOLUTION_LENGTH) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid contention resolution length\n",  __func__, m_uniqueId);
        return result;
    }
    UInt8 lcId = data[i];
    if (lcId != lc_contention_resolution) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, invalid lcId = %d\n",  __func__, m_uniqueId, lcId);
        return result;
    }
    // parse and validate
    ++i;
    UInt8 ueId = ((data[i] & 0x1f) << 3) | ((data[i+1] & 0xe0) >> 5);
    ++i;
    if (ueId != m_ueId) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, invalid parsed ueId = %d\n",  __func__, m_uniqueId, ueId);
        return result;
    }

    result = TRUE;

    return result;
}

// --------------------------------------------
void UESendRrcReestablishmentReq::handleCCCHMsg(UInt16 rrcMsgType) {
	if (rrcMsgType == 1) {
		m_state = RRC_REJ_RECVD;
		this->stopRRCSetupTimer();
		m_stsCounter->countSuccRejTest();
		m_stsCounter->countRRCRestabRej();
		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv RRC Connection Reestablishment Reject, change m_state to %d\n",  __func__, m_uniqueId, m_state);
	} else {
		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, Unexpected RRC msg, rrcMsgType = %d\n",  __func__, m_uniqueId, rrcMsgType);
		m_stsCounter->countTestFailure();
		m_state = WAIT_TERMINATING;
	}
}

