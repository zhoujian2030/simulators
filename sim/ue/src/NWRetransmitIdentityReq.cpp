/*
 * NWRetransmitIdentityReq.cpp
 *
 *  Created on: 2017-3-15
 *      Author: j.zhou
 */

#include "NWRetransmitIdentityReq.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif
#include "PhyMacAPI.h"
#include "UeScheduler.h"
#include "StsCounter.h"

using namespace ue;

#define MAX_RETRANSMIT_COUNT 8
//#define SEND_IDENTITY_RESP

// --------------------------------------------
NWRetransmitIdentityReq::NWRetransmitIdentityReq(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, phyMacAPI, stsCounter)
{
	m_numIdentityReqRetrans = 0;
}

// --------------------------------------------
NWRetransmitIdentityReq::~NWRetransmitIdentityReq() {

}

void NWRetransmitIdentityReq::resetChild() {
	m_numIdentityReqRetrans = 0;
}

// --------------------------------------------
void NWRetransmitIdentityReq::dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqProcessNum = %d, result = %d\n",  __func__,
        m_uniqueId, m_state, harqProcessNum, result);

    if (result == TRUE) {
        UInt32 msgLen = 0;
        FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getHarqBuffer();
        FAPI_harqIndication_st* pHarqInd = (FAPI_harqIndication_st *)&pL1Api->msgBody[0];

        // for both bundling mode and special bundling mode, UE send ack for all DL TB or nack for all DL TB
        if (firstAck) {
        	if (m_state != RRC_IDENTITY_REQ_RECVD || m_numIdentityReqRetrans > MAX_RETRANSMIT_COUNT) {
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
        	} else {
#if 0
        		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, Not send HARQ ACK, wait network retransmit identity request\n", __func__, m_uniqueId);
#else
        		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, Send HARQ NACK, wait network retransmit identity request\n", __func__, m_uniqueId);
				pL1Api->lenVendorSpecific = 0;
				pL1Api->msgId = PHY_UL_HARQ_INDICATION;

				pHarqInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
				pHarqInd->numOfHarq += 1;
				UInt32 harqHeaderLen = offsetof(FAPI_harqIndication_st, harqPduInfo);

				FAPI_tddHarqPduIndication_st* pTddHarqPduInd = (FAPI_tddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1];
				pTddHarqPduInd->rnti = m_rnti;
				pTddHarqPduInd->mode = tddAckNackFeedbackMode;  // 0: bundling, 1: multplexing, 2: special bundling
				pTddHarqPduInd->numOfAckNack += 1;
				pTddHarqPduInd->harqBuffer[0] = 4;
				pTddHarqPduInd->harqBuffer[1] = 0;

				msgLen += sizeof(FAPI_tddHarqPduIndication_st);
				pL1Api->msgLen += msgLen;

				m_phyMacAPI->addHarqDataLength(msgLen);
				if (pHarqInd->numOfHarq == 1) {
					m_phyMacAPI->addHarqDataLength(FAPI_HEADER_LENGTH + harqHeaderLen);
					pL1Api->msgLen += harqHeaderLen;
				}
#endif
				return;
        	}

            if (m_state == MSG4_RECVD) {
                m_state = MSG4_ACK_SENT;
                this->startRRCSetupTimer();
            } else if (m_state == RRC_SETUP_RECVD) {
            	m_state = RRC_SETUP_ACK_SENT;
            	this->setSfnSfForSR();
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

// --------------------------------------------
void NWRetransmitIdentityReq::rrcCallback(UInt32 msgType, RrcMsg* msg) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, msgType = %d\n",  __func__, m_uniqueId, msgType);

    switch (msgType) {
        case IDENTITY_REQUEST:
        {
            m_stsCounter->countIdentityRequestRecvd();
            if (m_state == RRC_IDENTITY_REQ_RECVD) {
            	this->m_stsCounter->countIdentityReqRetransmit();
            	m_numIdentityReqRetrans++;
            	if (m_numIdentityReqRetrans > MAX_RETRANSMIT_COUNT) {
#ifdef SEND_IDENTITY_RESP
            		m_state = RRC_CONNECTED;
            		m_triggerIdRsp = TRUE;
            		requestUlResource();
            		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv retransmit Identity Request, will send Identity response later\n",
            				__func__, m_uniqueId);
#else
            		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv retransmit Identity Request, will not send Identity response, wait RRC release\n",
            		            				__func__, m_uniqueId);
#endif
            	} else {
					LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv retransmit Identity Request, not send Identity response, m_numIdentityReqRetrans = %d\n",
							__func__, m_uniqueId, m_numIdentityReqRetrans);
            	}
            } else {
				m_state = RRC_IDENTITY_REQ_RECVD;
				LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Identity Request, not send Identity response\n",  __func__, m_uniqueId);
            }

            break;
        }

        case ATTACH_REJECT:
        {
            m_stsCounter->countAttachRejectRecvd();
            m_state = RRC_CONNECTED;
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Attach Reject\n",  __func__, m_uniqueId);
            break;
        }

        case RRC_RELEASE:
        {
            m_stsCounter->countRRCRelRecvd();
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv RRC Release, will free resource later\n",  __func__, m_uniqueId);
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change state from %d to %d\n",  __func__, m_uniqueId, m_state, RRC_RELEASING);
            m_state = RRC_RELEASING;
            break;
        }

        default:
        {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Unsupported msgType = %d\n",  __func__, m_uniqueId, msgType);
        }
    }
}

// --------------------------------------------
void NWRetransmitIdentityReq::rlcCallback(UInt8* statusPdu, UInt32 length) {
    if (m_state != RRC_IDENTITY_REQ_RECVD || m_numIdentityReqRetrans > MAX_RETRANSMIT_COUNT) {
    	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, need to send RLC status PDU, length = %d\n",  __func__, m_uniqueId, length);

        if (length != 2) {
            LOG_WARN(UE_LOGGER_NAME, "[%s], %s, Only support 2 bytes format RLC Status PDU now\n",  __func__, m_uniqueId);
            return;
        }
		m_triggerRlcStatusPdu = TRUE;
		requestUlResource();
		memcpy(&m_rlcStatusPdu, statusPdu, length);
    }
}



