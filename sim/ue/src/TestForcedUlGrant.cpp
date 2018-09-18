/*
 * TestForcedUlGrant.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: J.ZH
 */

#include "TestForcedUlGrant.h"
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
TestForcedUlGrant::TestForcedUlGrant(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, preamble, phyMacAPI, stsCounter)
{

}

// --------------------------------------------
TestForcedUlGrant::~TestForcedUlGrant() {

}

// --------------------------------------------
void TestForcedUlGrant::dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result) {
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
            if (m_state == MSG4_RECVD) {
                // TODO MAC will not check harq value for MSG4, just take all value as ACK
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

            pL1Api->msgLen += msgLen;

            m_phyMacAPI->addHarqDataLength(msgLen);
            if (pHarqInd->numOfHarq == 1) {
                m_phyMacAPI->addHarqDataLength(FAPI_HEADER_LENGTH + harqHeaderLen);
                pL1Api->msgLen += harqHeaderLen;
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

// --------------------------------------------
void TestForcedUlGrant::rrcCallback(UInt32 msgType, RrcMsg* msg) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, msgType = %d\n",  __func__, m_uniqueId, msgType);

    switch (msgType) {
        case IDENTITY_REQUEST:
        {
            m_stsCounter->countIdentityRequestRecvd();
//			m_triggerIdRsp = TRUE;
			LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Identity Request, will not send Identity Resp later\n",  __func__, m_uniqueId);

//			requestUlResource();

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

