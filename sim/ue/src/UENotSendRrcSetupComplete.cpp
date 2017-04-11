/*
 * UENotSendRrcSetupComplete.cpp
 *
 *  Created on: 2017-3-15
 *      Author: j.zhou
 */

#include "UENotSendRrcSetupComplete.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "../sysService/common/logger.h"
#endif
#include "PhyMacAPI.h"
#include "UeScheduler.h"
#include "StsCounter.h"

using namespace ue;

// --------------------------------------------
UENotSendRrcSetupComplete::UENotSendRrcSetupComplete(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, phyMacAPI, stsCounter)
{

}

// --------------------------------------------
UENotSendRrcSetupComplete::~UENotSendRrcSetupComplete() {

}

// --------------------------------------------
void UENotSendRrcSetupComplete::dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqProcessNum = %d, result = %d\n",  __func__,
        m_uniqueId, m_state, harqProcessNum, result);

    if (result == TRUE) {
        UInt32 msgLen = 0;
        FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getHarqBuffer();
        FAPI_harqIndication_st* pHarqInd = (FAPI_harqIndication_st *)&pL1Api->msgBody[0];

        // for both bundling mode and special bundling mode, UE send ack for all DL TB or nack for all DL TB
        if (firstAck) {
			LOG_INFO(UE_LOGGER_NAME, "[%s], %s, send HARQ ACK\n", __func__, m_uniqueId);
			pL1Api->lenVendorSpecific = 0;
			pL1Api->msgId = PHY_UL_HARQ_INDICATION;

			pHarqInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
			pHarqInd->numOfHarq += 1;
			UInt32 harqHeaderLen = offsetof(FAPI_harqIndication_st, harqPduInfo);

			FAPI_tddHarqPduIndication_st* pTddHarqPduInd = (FAPI_tddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
			pTddHarqPduInd->rnti = m_rnti;
			pTddHarqPduInd->mode = tddAckNackFeedbackMode;  // 0: bundling, 1: multplexing, 2: special bundling
			pTddHarqPduInd->numOfAckNack += 1;
			pTddHarqPduInd->harqBuffer[0] = ackFlag;
			pTddHarqPduInd->harqBuffer[1] = 0;

			msgLen += sizeof(FAPI_tddHarqPduIndication_st);
			pL1Api->msgLen += msgLen;

			m_phyMacAPI->addHarqDataLength(msgLen);
			if (pHarqInd->numOfHarq == 1) {
				m_phyMacAPI->addHarqDataLength(FAPI_HEADER_LENGTH + harqHeaderLen);
				pL1Api->msgLen += harqHeaderLen;
			}

            if (m_state == MSG4_RECVD) {
                m_state = MSG4_ACK_SENT;
                this->startRRCSetupTimer();
            } else if (m_state == RRC_SETUP_RECVD) {
            	m_state = RRC_RELEASING;//WAIT_TERMINATING;
            	//this->m_stsCounter->countTestSuccess();
            	LOG_INFO(UE_LOGGER_NAME, "[%s], %s, Not send RRC Setup Complete, wait RRC terminated, change state to %d\n", __func__, m_uniqueId, m_state);
            } else if (m_state == RRC_REJ_RECVD) {
            	m_state = WAIT_TERMINATING;
            } else {
                // TODO
            }
        } else {
            FAPI_tddHarqPduIndication_st* pTddHarqPduInd = (FAPI_tddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
            pTddHarqPduInd->numOfAckNack += 1;
        }

        // it counts number of ACK needs to sent for DL TB,
        // not counts the actual harq messages (FAPI_harqIndication_st) sents
        m_stsCounter->countHarqAckSent();
    }
}
