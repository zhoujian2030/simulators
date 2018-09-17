/*
 * NWRetransmitRrcSetup.cpp
 *
 *  Created on: 2017-3-15
 *      Author: j.zhou
 */

#include "NWRetransmitRrcSetup.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif
#include "PhyMacAPI.h"
#include "UeScheduler.h"
#include "StsCounter.h"

using namespace ue;

#define MAX_RETRANSMIT_COUNT 4

// --------------------------------------------
NWRetransmitRrcSetup::NWRetransmitRrcSetup(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, preamble, phyMacAPI, stsCounter)
{
    m_numRRCSetupRetrans = 0;
}

// --------------------------------------------
NWRetransmitRrcSetup::~NWRetransmitRrcSetup() {

}

// --------------------------------------------
void NWRetransmitRrcSetup::allocateDlHarqCallback(UInt16 harqProcessNum, BOOL result) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqProcessNum = %d, result = %d\n",  __func__,
        m_uniqueId, m_state, harqProcessNum, result);

    if (result == TRUE) {
        if (m_state == MSG3_SENT) {
            m_state = MSG4_DCI_RECVD;
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, recv Contention resolution DCI pdu, change state to %d\n",  __func__, m_uniqueId, m_state);
        } else if (m_state == MSG4_ACK_SENT || m_state == RRC_SETUP_RETRANSMIT) {
        	if (RRC_SETUP_RETRANSMIT == m_state) {
#if 0
        		this->m_stsCounter->countRRCSetupRetransmit();
        		m_state = WAIT_TERMINATING;
        		this->m_stsCounter->countTestFailure();
#else            
                m_numRRCSetupRetrans++;
                this->m_stsCounter->countRRCSetupRetransmit();
                m_state = RRC_SETUP_DCI_RECVD;
#endif
        	} else {
        		m_state = RRC_SETUP_DCI_RECVD;
        	}
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, recv RRC setup DCI pdu, change state to %d\n",  __func__, m_uniqueId, m_state);
        } else {
            // TODO
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, other DL DCI pdu\n",  __func__, m_uniqueId);
        }
    } else {
        // if fail to process DL DCI when RRC connection not established, terminate it
        if (m_state != RRC_CONNECTED) {
            m_state = WAIT_TERMINATING;
            this->m_stsCounter->countAllocDlHarqFailure();
            this->m_stsCounter->countTestFailure();
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, allocate DL HARQ process failure, change m_state to %d\n",  __func__, m_uniqueId, m_state);
        }
    }
}

// --------------------------------------------
void NWRetransmitRrcSetup::dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqProcessNum = %d, result = %d\n",  __func__,
        m_uniqueId, m_state, harqProcessNum, result);

    if (result == TRUE) {
        UInt32 msgLen = 0;
        FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getHarqBuffer();
        FAPI_harqIndication_st* pHarqInd = (FAPI_harqIndication_st *)&pL1Api->msgBody[0];

        // for both bundling mode and special bundling mode, UE send ack for all DL TB or nack for all DL TB
        if (firstAck) {
        	if ((m_state != RRC_SETUP_RECVD) || (m_numRRCSetupRetrans >= MAX_RETRANSMIT_COUNT)) {
				LOG_INFO(UE_LOGGER_NAME, "[%s], %s, send HARQ ACK\n", __func__, m_uniqueId);
				pL1Api->lenVendorSpecific = 0;
				pL1Api->msgId = PHY_UL_HARQ_INDICATION;

				pHarqInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
				pHarqInd->numOfHarq += 1;
				UInt32 harqHeaderLen = offsetof(FAPI_harqIndication_st, harqPduInfo);
#ifdef TDD_CONFIG
				FAPI_tddHarqPduIndication_st* pTddHarqPduInd = (FAPI_tddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
				pTddHarqPduInd->rnti = m_rnti;
				pTddHarqPduInd->mode = tddAckNackFeedbackMode;  // 0: bundling, 1: multplexing, 2: special bundling
				pTddHarqPduInd->numOfAckNack += 1;
				pTddHarqPduInd->harqBuffer[0] = ackFlag;
				pTddHarqPduInd->harqBuffer[1] = 0;

				msgLen += sizeof(FAPI_tddHarqPduIndication_st);
#else
				FAPI_fddHarqPduIndication_st* pFddHarqPduInd = (FAPI_fddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
				pFddHarqPduInd->rnti = m_rnti;
				pFddHarqPduInd->harqTB1 = ackFlag;
				pFddHarqPduInd->harqTB2 = 0;
				msgLen += sizeof(FAPI_fddHarqPduIndication_st);
#endif
				pL1Api->msgLen += msgLen;

				m_phyMacAPI->addHarqDataLength(msgLen);
				if (pHarqInd->numOfHarq == 1) {
					m_phyMacAPI->addHarqDataLength(FAPI_HEADER_LENGTH + harqHeaderLen);
					pL1Api->msgLen += harqHeaderLen;
				}
        	}

            if (m_state == MSG4_RECVD) {
                m_state = MSG4_ACK_SENT;
                this->startRRCSetupTimer();
            } else if (m_state == RRC_SETUP_RECVD) {
                if (m_numRRCSetupRetrans < MAX_RETRANSMIT_COUNT) {
                    m_state = RRC_SETUP_RETRANSMIT;
#if 0
                    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, Not send HARQ ACK, wait network restransmit, change state to %d\n", __func__, m_uniqueId, m_state);
#else
                    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, send HARQ NACK\n", __func__, m_uniqueId);
                    pL1Api->lenVendorSpecific = 0;
                    pL1Api->msgId = PHY_UL_HARQ_INDICATION;

                    pHarqInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
                    pHarqInd->numOfHarq += 1;
                    UInt32 harqHeaderLen = offsetof(FAPI_harqIndication_st, harqPduInfo);

#ifdef TDD_CONFIG
                    FAPI_tddHarqPduIndication_st* pTddHarqPduInd = (FAPI_tddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
                    pTddHarqPduInd->rnti = m_rnti;
                    pTddHarqPduInd->mode = tddAckNackFeedbackMode;  // 0: bundling, 1: multplexing, 2: special bundling
                    pTddHarqPduInd->numOfAckNack += 1;
                    pTddHarqPduInd->harqBuffer[0] = 4;
                    pTddHarqPduInd->harqBuffer[1] = 0;

                    msgLen += sizeof(FAPI_tddHarqPduIndication_st);
#else
    				FAPI_fddHarqPduIndication_st* pFddHarqPduInd = (FAPI_fddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
    				pFddHarqPduInd->rnti = m_rnti;
    				pFddHarqPduInd->harqTB1 = 4;
    				pFddHarqPduInd->harqTB2 = 0;
    				msgLen += sizeof(FAPI_fddHarqPduIndication_st);
#endif

                    pL1Api->msgLen += msgLen;

                    m_phyMacAPI->addHarqDataLength(msgLen);
                    if (pHarqInd->numOfHarq == 1) {
                        m_phyMacAPI->addHarqDataLength(FAPI_HEADER_LENGTH + harqHeaderLen);
                        pL1Api->msgLen += harqHeaderLen;
                    }
#endif
            	    return;
                } else {
                    m_state = RRC_SETUP_ACK_SENT;
                    m_numRRCSetupRetrans = 0;
                    this->setSfnSfForSR();
                }
            } else if (m_state == RRC_REJ_RECVD) {
            	m_state = WAIT_TERMINATING;
            } else {
                // TODO
            }
        } else {
#ifdef TDD_CONFIG
            FAPI_tddHarqPduIndication_st* pTddHarqPduInd = (FAPI_tddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
            pTddHarqPduInd->numOfAckNack += 1;
#else
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, TODO send HARQ ACK for harqTB2\n", __func__, m_uniqueId);
            FAPI_fddHarqPduIndication_st* pFddHarqPduInd = (FAPI_fddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
            pFddHarqPduInd->rnti = m_rnti;
            pFddHarqPduInd->harqTB2 = ackFlag;
#endif
        }

        // it counts number of ACK needs to sent for DL TB,
        // not counts the actual harq messages (FAPI_harqIndication_st) sents
        m_stsCounter->countHarqAckSent();
    }
}



