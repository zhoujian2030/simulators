/*
 * UeScheduler.cpp
 *
 *  Created on: Nov 07, 2016
 *      Author: j.zhou
 */



#include "UeScheduler.h"
#include "UeTerminal.h"
#include "PhyMacAPI.h"
#include "StsCounter.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif

#include "NWRetransmitRrcSetup.h"
#include "UENotSendRrcSetupComplete.h"
#include "NWRetransmitIdentityReq.h"
#include "UESuspending.h"
#include "UENotSendRlcAck.h"
#include "UESendRrcReestablishmentReq.h"
#include "UeNotSendRlcAndHarqAck.h"
#include "UENotSendIdentityResp.h"
#include "UENotSendHarqAckForMsg4.h"

using namespace ue;
using namespace std;

// ----------------------------------------
NodePool::NodePool(UInt32 size) {
    m_head = new Node();
    if (size > 0) {
        size--;
    }

    LOG_TRACE(UE_LOGGER_NAME, "[%s], m_head = %p\n", __func__, m_head);

    Node* node = 0;
    while (size--) {
        node = new Node();
        if (node == 0) {
        	LOG_DBG(UE_LOGGER_NAME, "[%s], fail to create new node, size = %d\n", __func__, size);
        	break;
        }
        node->next = m_head;
        m_head = node;
    }
}

// ----------------------------------------
NodePool::~NodePool() {

}

// ----------------------------------------
Node* NodePool::getNode() {
	Node* retNode = m_head;
	LOG_TRACE(UE_LOGGER_NAME, "[%s], retNode = %p\n", __func__, retNode);
	if (m_head != 0) {
		m_head = m_head->next;
		retNode->next = 0;
		retNode->tail = 0;
	}
	return retNode;
}

// ----------------------------------------
void NodePool::freeNode(Node* node) {
	if (node != 0) {
		node->next = m_head;
		m_head = node;
	}
}

// ----------------------------------------
UeScheduler::UeScheduler(PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: m_sfn(0), m_sf(0)
{
	LOG_TRACE(UE_LOGGER_NAME, "[%s], Entry\n", __func__);

	m_numUe = 1;

    // The ueId and ra-rnti value is in range 1~MAX_UE_SUPPORTED
    m_ueList = new UeTerminal*[MAX_UE_SUPPORTED];
#if 1
    for (UInt32 i=0; i<MAX_UE_SUPPORTED; i++) {
    	 m_ueList[i] = new UeTerminal(i+1, i+1, i+1, phyMacAPI, stsCounter);
//   	    m_ueList[i] = new NWRetransmitRrcSetup(i+1, i+1, i+1, phyMacAPI, stsCounter);
//       m_ueList[i] = new UENotSendRrcSetupComplete(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new NWRetransmitIdentityReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UESuspending(i+1, i+1, phyMacAPI, i+1, stsCounter);
//        m_ueList[i] = new UENotSendRlcAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
//		 m_ueList[i] = new UESendRrcReestablishmentReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	m_ueList[i] = new UeTerminal(i+1, 3, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UeNotSendRlcAndHarqAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	 m_ueList[i] = new UENotSendIdentityResp(i+1, i+1, i+1, phyMacAPI, stsCounter);
//		 m_ueList[i] = new UENotSendHarqAckForMsg4(i+1, i+1, i+1, phyMacAPI, stsCounter);

    	 m_ueList[i]->updateConfig(0);
    }
#else

    UInt16 count = 15;
    for (UInt32 i=0; i<count; i++) {
    	 m_ueList[i] = new UeTerminal(i+1, i+1, i+1, phyMacAPI, stsCounter);
//   	    m_ueList[i] = new NWRetransmitRrcSetup(i+1, i+1, i+1, phyMacAPI, stsCounter);
//       m_ueList[i] = new UENotSendRrcSetupComplete(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new NWRetransmitIdentityReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UESuspending(i+1, i+1, phyMacAPI, i+1, stsCounter);
//        m_ueList[i] = new UENotSendRlcAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
		// m_ueList[i] = new UESendRrcReestablishmentReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	m_ueList[i] = new UeTerminal(i+1, 3, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UeNotSendRlcAndHarqAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	 m_ueList[i] = new UENotSendIdentityResp(i+1, i+1, i+1, phyMacAPI, stsCounter);

    	 m_ueList[i]->updateConfig(0);
    }

    for (UInt32 i=count; i<(count + 1); i++) {
    	 m_ueList[i] = new UeTerminal(i+1, i+1, i+1, phyMacAPI, stsCounter);
//   	    m_ueList[i] = new NWRetransmitRrcSetup(i+1, i+1, i+1, phyMacAPI, stsCounter);
//       m_ueList[i] = new UENotSendRrcSetupComplete(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new NWRetransmitIdentityReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UESuspending(i+1, i+1, phyMacAPI, i+1, stsCounter);
//        m_ueList[i] = new UENotSendRlcAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
		// m_ueList[i] = new UESendRrcReestablishmentReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	m_ueList[i] = new UeTerminal(i+1, 3, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UeNotSendRlcAndHarqAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	 m_ueList[i] = new UENotSendIdentityResp(i+1, i+1, i+1, phyMacAPI, stsCounter);

    	 m_ueList[i]->updateConfig(0);
    }

    count++;
    for (UInt32 i=count; i<(count + 3); i++) {
    	 m_ueList[i] = new UeTerminal(i+1, i+1, i+1, phyMacAPI, stsCounter);
//   	    m_ueList[i] = new NWRetransmitRrcSetup(i+1, i+1, i+1, phyMacAPI, stsCounter);
//       m_ueList[i] = new UENotSendRrcSetupComplete(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new NWRetransmitIdentityReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UESuspending(i+1, i+1, phyMacAPI, i+1, stsCounter);
//        m_ueList[i] = new UENotSendRlcAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
		// m_ueList[i] = new UESendRrcReestablishmentReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	m_ueList[i] = new UeTerminal(i+1, 3, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UeNotSendRlcAndHarqAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	 m_ueList[i] = new UENotSendIdentityResp(i+1, i+1, i+1, phyMacAPI, stsCounter);

    	 m_ueList[i]->updateConfig(0);
    }

    count += 3;
    for (UInt32 i=count; i<(count + 3); i++) {
//    	 m_ueList[i] = new UeTerminal(i+1, i+1, i+1, phyMacAPI, stsCounter);
//   	    m_ueList[i] = new NWRetransmitRrcSetup(i+1, i+1, i+1, phyMacAPI, stsCounter);
//       m_ueList[i] = new UENotSendRrcSetupComplete(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new NWRetransmitIdentityReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UESuspending(i+1, i+1, phyMacAPI, i+1, stsCounter);
//        m_ueList[i] = new UENotSendRlcAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
		 m_ueList[i] = new UESendRrcReestablishmentReq(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	m_ueList[i] = new UeTerminal(i+1, 3, i+1, phyMacAPI, stsCounter);
//        m_ueList[i] = new UeNotSendRlcAndHarqAck(i+1, i+1, i+1, phyMacAPI, stsCounter);
//    	 m_ueList[i] = new UENotSendIdentityResp(i+1, i+1, i+1, phyMacAPI, stsCounter);

    	 m_ueList[i]->updateConfig(0);
    }
#endif

    for (UInt32 i=0; i<DL_MSG_CONTAINER_SIZE; i++) {
        m_dlMsgBufferContainer[i].length = 0;
    }
    m_ulCfgMsg.length = 0;

    m_nodePool = new NodePool(DL_MSG_CONTAINER_SIZE);
    m_dlDataNodeHead = 0;
    m_dlConfigNodeHead = 0;
    m_ueConfigNodeHead = 0;
    m_hiDci0NodeHead = 0;
}

// ----------------------------------------
UeScheduler::~UeScheduler() {

}

// ----------------------------------------
void UeScheduler::updateUeConfig(UInt32 numUe, UInt32 numAccessCount) {
	for (UInt32 i=0; i<MAX_UE_SUPPORTED; i++) {
		if (i < numUe) {
			m_ueList[i]->updateConfig(numAccessCount);
		} else {
			m_ueList[i]->updateConfig(0);
//			m_ueList[i]->reset();
		}
	}

	if (numUe < MAX_UE_SUPPORTED) {
		m_numUe = numUe;
	} else {
		m_numUe = MAX_UE_SUPPORTED;
	}
}

// ----------------------------------------
void UeScheduler::updateSfnSf(UInt16 sfn, UInt8 sf) {
	m_prevSfn = m_sfn;
	m_prevSf = m_sf;

	UInt8 expectedSf = (m_sf + 1) % 10;
	UInt16 expectedSfn = (m_sfn + (m_sf + 1) / 10) % 1024;
	if ((expectedSf != sf) || (expectedSfn != sfn)) {
		LOG_ERROR(UE_LOGGER_NAME, "[%s], sfnsf not consecutive!\n", __func__);
	}

    m_sfn = sfn;
    m_sf  = sf;
}

// ----------------------------------------
extern BOOL gIsCliCommand;
#define GENERATE_SUBFRAME_SFNSF(sfn,sf) ( ( (sfn) << 4) | ( (sf) & 0xf) )
void UeScheduler::processDlData(UInt8* buffer, SInt32 length) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], Entry \n", __func__);

    gIsCliCommand = FALSE;

    FAPI_l1ApiMsg_st *pL1Api = (FAPI_l1ApiMsg_st *)buffer;

    if (pL1Api->msgId == PHY_UL_CONFIG_REQUEST) {
        memcpy(m_ulCfgMsg.data, buffer, length);
        m_ulCfgMsg.length = length;
        LOG_DBG(UE_LOGGER_NAME, "[%s], save PHY_UL_CONFIG_REQUEST, in %d.%d\n", __func__, m_sfn, m_sf);
    } else {
        static UInt16 historyNodes = 0;

        Node* node = m_nodePool->getNode();
        if (node == 0) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get free node, historyNodes = %d\n", __func__, historyNodes);
            return;
        }

        historyNodes++;
        BOOL foundFreeBuffer = FALSE;
        for (UInt32 i=0; i<DL_MSG_CONTAINER_SIZE; i++) {

            if (m_dlMsgBufferContainer[i].length == 0) {
                node->buffer = (void*)&m_dlMsgBufferContainer[i];
                memcpy(m_dlMsgBufferContainer[i].data, buffer, length);
                m_dlMsgBufferContainer[i].length = length;

                foundFreeBuffer = TRUE;

                switch (pL1Api->msgId) {
					case PHY_DL_CONFIG_REQUEST:
					{
						if (m_dlConfigNodeHead == 0) {
							m_dlConfigNodeHead = node;
							m_dlConfigNodeHead->tail = node;
						} else {
							m_dlConfigNodeHead->tail->next = node;
							m_dlConfigNodeHead->tail = node;
						}
						break;
					}

					case PHY_DL_TX_REQUEST:
					{
						if (m_dlDataNodeHead == 0) {
							m_dlDataNodeHead = node;
							m_dlDataNodeHead->tail = node;
						} else {
							m_dlDataNodeHead->tail->next = node;
							m_dlDataNodeHead->tail = node;
						}
						break;
					}

					case PHY_DL_HI_DCI0_REQUEST:
					{
						if (m_hiDci0NodeHead == 0) {
							m_hiDci0NodeHead = node;
							m_hiDci0NodeHead->tail = node;
						} else {
							m_hiDci0NodeHead->tail->next = node;
							m_hiDci0NodeHead->tail = node;
						}
						break;
					}

#ifdef OS_LINUX
					case PHY_DELETE_UE_REQUEST:
					{
				        if (pL1Api->msgId == PHY_DELETE_UE_REQUEST) {
				            UInt16* pSfnSf = (UInt16*)&pL1Api->msgBody[0];
				            UInt8 sf = (m_sf + 2) % 10;
				            UInt16 sfn = (m_sfn + (m_sf + 2)/10) % 1024;
				            *pSfnSf = GENERATE_SUBFRAME_SFNSF(sfn, sf);
				            LOG_DBG(UE_LOGGER_NAME, "[%s], recv PHY_DELETE_UE_REQUEST in %d.%d, will handle it in %d.%d\n", __func__, m_sfn, m_sf, sfn, sf);
				        }
#else
					case PHY_UE_CONFIG_REQUEST:
					{
#endif
						LOG_INFO(UE_LOGGER_NAME, "[%s], PHY_UE_CONFIG_REQUEST, node = %p\n", __func__, node);
						if (m_ueConfigNodeHead == 0) {
							m_ueConfigNodeHead = node;
							m_ueConfigNodeHead->tail = node;
						} else {
							m_ueConfigNodeHead->tail->next = node;
							m_ueConfigNodeHead->tail = node;
						}
						break;
					}

					default:
					{
						LOG_ERROR(UE_LOGGER_NAME, "[%s], unsupport msgId = %d\n", __func__, pL1Api->msgId);
						m_dlMsgBufferContainer[i].length = 0;
						m_nodePool->freeNode(node);
						gIsCliCommand = TRUE;
						break;
					}
                }

                break;
            }
        }  

        if (!foundFreeBuffer) {
        	LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get free buffer, free the node = %p\n", __func__, node);
        	m_nodePool->freeNode(node);
        }
    }
}

// ----------------------------------------
void UeScheduler::handleUeConfigReq(FAPI_phyUeConfigRequest_st* pUeConfigReq) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], cfgMode = %d\n", __func__, pUeConfigReq->cfgMode);

	UInt16 rnti = 0;
	UInt16 srConfigIndex = 0;
	BOOL foundRnti = FALSE;
	BOOL foundSRConfigIndex = FALSE;

	UInt8 tlvIndex=0;
	for(tlvIndex=0; tlvIndex<pUeConfigReq->numOfTlv; tlvIndex++) {

		if (foundRnti && foundSRConfigIndex) {
			break;
		}

		switch(pUeConfigReq->configtlvs[tlvIndex].tag) {

			case FAPI_RNTI:
			{
				rnti = pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.rnti;
				LOG_INFO(UE_LOGGER_NAME, "[%s], rnti = 0x%x\n", __func__, rnti)
				foundRnti = TRUE;
				break;
			}

			case FAPI_SR_CONFIG_INDEX:
			{
				LOG_DBG(UE_LOGGER_NAME, "[%s], srConfigIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srConfigIndex);
				srConfigIndex = pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srConfigIndex;
				foundSRConfigIndex = TRUE;
				break;
			}

			case FAPI_TRANSMISSION_MODE_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], transModePresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.transModePresent);
				break;
			}
			case FAPI_TRANSMISSION_MODE:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], transMode = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.transMode);
				break;
			}
			case FAPI_CQI_CONFIG_INFO_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], cqiCfgPresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.cqiCfgPresent);
				break;
			}
			case FAPI_APERIODIC_CQI_CFG_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], aperiodicCqiCfgPresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.aperiodicCqiCfgPresent);
				break;
			}
			case FAPI_APERIODIC_CQI_CFG_EN:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], aperiodicCqiCfgEn = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.aperiodicCqiCfgEn);
				break;
			}
			case FAPI_APERIODIC_CQI_REPORT_MODE:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], aperiodicCqiReportMode = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.aperiodicCqiReportMode);
				break;
			}
			case FAPI_PDSCH_EPRE_TO_UE_RS_RATIO:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], pdschEpreToUeRsRatio = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.pdschEpreToUeRsRatio);
				break;
			}
			case FAPI_PERIODIC_CQI_CFG_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], periodicCqiCfgPresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.periodicCqiCfgPresent);
				break;
			}
			case FAPI_PERIODIC_CQI_CFG_EN:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], periodicCqiCfgEn = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.periodicCqiCfgEn);
				break;
			}
			case FAPI_CQI_PUCCH_RESOURCE_INDEX:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], cqiPucchResourceIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.cqiPucchResourceIndex);
				break;
			}
			case FAPI_CQI_PMI_CONFIG_INDEX:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], cqiPmiConfigIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.cqiPmiConfigIndex);
				break;
			}
			case FAPI_CQI_RI_CONFIG_INDEX:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], cqiRiConfigIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.cqiRiConfigIndex);
				break;
			}
			case FAPI_SIMULTANEOUS_ACK_NACK_AND_CQI:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], simultaneousAckNackAndCqi = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.simultaneousAckNackAndCqi);
				break;
			}
			case FAPI_CQI_FORMAT_INDICATOR_PERIODIC:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], cqiformatIndicatorPeriodic = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.cqiformatIndicatorPeriodic);
				break;
			}
			case FAPI_CQI_MASK_V920_EN:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], cqiMask_v920_En = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.cqiMask_v920_En);
				break;
			}
			case FAPI_CQI_MASK_V920:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], cqiMask_v920 = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.cqiMask_v920);
				break;
			}
			case FAPI_PMI_RI_REPORT_V920_EN:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], pmiRiReport_V920_En = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.pmiRiReport_V920_En);
				break;
			}
			case FAPI_PMI_RI_REPORT_V920:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], pmiRiReport_V920 = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.pmiRiReport_V920);
				break;
			}
			case FAPI_K:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], cqiformatIndicatorPeriodic_subband_k = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.cqiformatIndicatorPeriodic_subband_k);
				break;
			}
			case FAPI_ACK_NACK_CONFIG_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], ackNAckConfigPresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.ackNAckConfigPresent);
				break;
			}
			case FAPI_AN_N1_PUCCH_AN_REP:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], anN1PUCCHANRep = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.anN1PUCCHANRep);
				break;
			}
			case FAPI_TDD_ACK_NACK_FEEDBACK_MODE_EN:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], tddAckNackFeedbackModeEn = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.tddAckNackFeedbackModeEn);
				break;
			}
			case FAPI_TDD_ACK_NACK_FEEDBACK_MODE:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], tddAckNackFeedbackMode = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.tddAckNackFeedbackMode);
				break;
			}
			case FAPI_SRS_CONFIG_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srsConfigPresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srsConfigPresent);
				break;
			}
			case FAPI_FREQ_DOMAIN_POSITION:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], freqDomainPosition = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.freqDomainPosition);
				break;
			}
			case FAPI_SRS_CONFIG_INDEX:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srsConfigIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srsConfigIndex);
				break;
			}
			case FAPI_SRS_CYCLIC_SHIFT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], soundingRefCyclicShift = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.soundingRefCyclicShift);
				break;
			}
			case FAPI_SRS_DURATION:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srsDuration = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srsDuration);
				break;
			}
			case FAPI_SRS_HOPPING_BANDWIDTH:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srsHoppingBandwidth = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srsHoppingBandwidth);
				break;
			}
			case FAPI_SRS_BANDWIDTH:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srSBandWidth = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srSBandWidth);
				break;
			}
			case FAPI_SRS_EN:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srsConfigEn = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srsConfigEn);
				break;
			}
			case FAPI_SRS_TRANSMISSION_COMB:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], txComb = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.txComb);
				break;
			}
			case FAPI_SR_CONFIG_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srConfigPresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srConfigPresent);
				break;
			}
			case FAPI_SR_PUCCH_RESOURCE_INDEX:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srPucchResourceIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srPucchResourceIndex);
				break;
			}
			case FAPI_SR_EN:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], srConfigEn = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.srConfigEn);
				break;
			}
			case FAPI_SPS_DL_CONFIG_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], spsDlConfigPresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.spsDlConfigPresent);
				break;
			}
			case FAPI_SPS_DL_EN:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], spsDlEn = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.spsDlEn);
				break;
			}
			case FAPI_SPS_DL_CONFIG_SCHED_INTERVAL:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], spsDlConfigSchedInterval = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.spsDlConfigSchedInterval);
				break;
			}
			case FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT0:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], spsDln1PUCCHANPersistent0 = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.spsDln1PUCCHANPersistent0)
				break;
			}
			case FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT1:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], spsDln1PUCCHANPersistent1 = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.spsDln1PUCCHANPersistent1);
				break;
			}
			case FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT2:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], spsDln1PUCCHANPersistent2 = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.spsDln1PUCCHANPersistent2);
				break;
			}
			case FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT3:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], spsDln1PUCCHANPersistent3 = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.spsDln1PUCCHANPersistent3);
				break;
			}
			case FAPI_PUSCH_CONFIG_INFO_PRESENT:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], uePuschCfgPresent = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.uePuschCfgPresent);
				break;
			}
			case FAPI_PUSCH_CONFIG_BETAOFFSET_ACK_INDEX:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], betaOffsetACKIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.betaOffsetACKIndex);
				break;
			}
			case FAPI_PUSCH_CONFIG_BETAOFFSET_RI_INDEX:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], betaOffsetRIIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.betaOffsetRIIndex);
				break;
			}
			case FAPI_PUSCH_CONFIG_BETAOFFSET_CQI_INDEX:
			{
				LOG_TRACE(UE_LOGGER_NAME, "[%s], betaOffsetCQIIndex = %d\n", __func__, pUeConfigReq->configtlvs[tlvIndex].ueConfigParam.betaOffsetCQIIndex);
				break;
			}
			default:
			{
				LOG_DBG(UE_LOGGER_NAME,
						"[%s], pUeConfigReq->configtlvs[%d].tag[%d] is invalid.\n",
						__func__, tlvIndex, pUeConfigReq->configtlvs[tlvIndex].tag);
				break;
			}
		}
	}

	if (pUeConfigReq->cfgMode == 1) {
		handleCreateUeReq(pUeConfigReq->ueIndex, rnti, srConfigIndex);
		m_ueIndexRntiMap[pUeConfigReq->ueIndex] = rnti;
//		LOG_INFO(UE_LOGGER_NAME, "[%s], cfgMode = 1, rnti = %d, ueIndex = %d\n", __func__, rnti, pUeConfigReq->ueIndex);
	} else if (pUeConfigReq->cfgMode == 3){
		map<UInt16, UInt16>::iterator it = m_ueIndexRntiMap.find(pUeConfigReq->ueIndex);
		if (it != m_ueIndexRntiMap.end()) {
			rnti = it->second;
//			LOG_INFO(UE_LOGGER_NAME, "[%s], cfgMode = 3, rnti = %d, ueIndex = %d\n", __func__, rnti, pUeConfigReq->ueIndex);
			handleDeleteUeReq(rnti);
		} else {
			LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueIndex = %d\n", __func__, pUeConfigReq->ueIndex);
		}
	} else {
		LOG_ERROR(UE_LOGGER_NAME, "[%s], unsupported cfgMode = %d\n", __func__, pUeConfigReq->cfgMode);
	}
}

// ----------------------------------------
void UeScheduler::handleCreateUeReq(UInt16 ueIndex, UInt16 rnti, UInt16 srConfigIndex) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], create rnti = %d, ueIndex = %d\n", __func__, rnti, ueIndex);

    if (rnti <= m_maxRaRntiUeId) {
        // For RAR, rnti is ra-rnti, which is the same as ueId
        m_ueList[rnti-1]->handleCreateUeReq(srConfigIndex);
    } else {
        // other msg, c-rnti
        map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
        if (it != m_rntiUeIdMap.end()) {
            UInt8 ueId = it->second;
            if (ueId <= m_maxRaRntiUeId) {
                m_ueList[ueId-1]->handleCreateUeReq(srConfigIndex);
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
            }
        } else {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get ueId by rnti = %d\n", __func__, rnti);
        }
    }
}

// ----------------------------------------
void UeScheduler::handleDeleteUeReq(UInt16 rnti) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], delete rnti = %d\n", __func__, rnti);

    if (rnti <= m_maxRaRntiUeId) {
        // For RAR, rnti is ra-rnti, which is the same as ueId
        m_ueList[rnti-1]->handleDeleteUeReq();
    } else {
        // other msg, c-rnti
        map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
        if (it != m_rntiUeIdMap.end()) {
            UInt8 ueId = it->second;
            if (ueId <= m_maxRaRntiUeId) {
                m_ueList[ueId-1]->handleDeleteUeReq();
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
            }
        } else {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get ueId by rnti = %d\n", __func__, rnti);
        }
    }
}

// ----------------------------------------
BOOL UeScheduler::schedule() {
    m_pduIndexUeIdMap.clear();

    if ( !((m_prevSfn == m_sfn) && (m_prevSf == m_sf)) ) {
		int numUeSchedule = MAX_UE_SUPPORTED;
		BOOL isScheduling = FALSE;
		for(int i=0; i<numUeSchedule; i++) {
			if (m_ueList[i] != 0) {
				if (!m_ueList[i]->schedule(m_sfn, m_sf, this)) {
//					LOG_WARN(UE_LOGGER_NAME, "[%s], UE %d is not scheduling\n", __func__, i);
//					m_ueList[i]->showConfig();
//					delete m_ueList[i];
//					m_ueList[i] = 0;
				} else {
					isScheduling = TRUE;
				}
			}
		}

		if (!isScheduling) {
			return FALSE;
		}
    } else {
    	LOG_WARN(UE_LOGGER_NAME, "[%s], sfnSf is not changed\n", __func__);
    }

    processData();

    return TRUE;
}

// ----------------------------------------
BOOL UeScheduler::validateSfnSf(BOOL isULCfg, UInt16 sfn, UInt8 sf) {
    UInt8 delay = DL_PHY_DELAY;
    if (isULCfg) {
        delay = UL_PHY_DELAY;
    }
    UInt32 curTick = m_sfn * 10 + m_sf;
    UInt16 provTick = sfn * 10 + sf;
    UInt8 delta;
    if (provTick < curTick) {
        delta = provTick + 10240 - curTick;
    } else {
        delta = provTick - curTick;
    }

    if (delta > delay) {
        return FALSE;
    } else {
        return TRUE;
    }        
}

// ----------------------------------------
void UeScheduler::processData() {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], Entry\n", __func__);

	// --------------------------------------------------------
    // process UL Cfg
	// TODO if more than one, need to save in node list
    if (m_ulCfgMsg.length > 0) {
        FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ulCfgMsg.data;
        FAPI_ulConfigRequest_st *pUlConfigReq = (FAPI_ulConfigRequest_st *)&pL1Api->msgBody[0];        
        UInt8 sf  = pUlConfigReq->sfnsf & 0x000f;
        UInt16 sfn  = (pUlConfigReq->sfnsf & 0xfff0) >> 4;
        LOG_DBG(UE_LOGGER_NAME, "[%s], handle msgId = 0x%02x in %d.%d, the provision sfnsf is %d.%d\n", __func__,
            pL1Api->msgId, m_sfn, m_sf, sfn, sf);
 
        if (!validateSfnSf(TRUE, sfn, sf) || (sf == m_sf && sfn == m_sfn)) {
            if(pUlConfigReq->ulConfigLen == (pL1Api->msgLen + pL1Api->lenVendorSpecific)) {
                handleUlConfigReq(pUlConfigReq);
            }
            m_ulCfgMsg.length = 0;
        }
    }

    // --------------------------------------------------------
    // process DL Cfg
    Node* node = m_dlConfigNodeHead;
    Node* prevNode = m_dlConfigNodeHead;
    DlMsgBuffer* msgBuffer = 0;
    while (node != 0) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], node = %p\n", __func__, node);

        msgBuffer = (DlMsgBuffer*)node->buffer;
        UInt16 msgId = 0;
        FAPI_l1ApiMsg_st *pL1Api = (FAPI_l1ApiMsg_st *)msgBuffer->data;
        msgId = pL1Api->msgId;

        FapiL1MsgHead* pMsgHead = (FapiL1MsgHead*)&pL1Api->msgBody[0];
        UInt8 sf  = pMsgHead->sfnsf & 0x000f;
        UInt16 sfn  = (pMsgHead->sfnsf & 0xfff0) >> 4;
        LOG_DBG(UE_LOGGER_NAME, "[%s], handle PHY_DL_CONFIG_REQUEST in %d.%d, the provision sfnsf is %d.%d\n", __func__,
            m_sfn, m_sf, sfn, sf);

        if (!validateSfnSf(FALSE, sfn, sf) || (sf == m_sf && sfn == m_sfn)) {
			FAPI_dlConfigRequest_st *pDlConfigReq = (FAPI_dlConfigRequest_st *)&pL1Api->msgBody[0];
			if (pDlConfigReq->length == (pL1Api->msgLen + pL1Api->lenVendorSpecific)) {
				handleDlConfigReq(pDlConfigReq);
			}

            // free the buffer and node
            msgBuffer->length = 0;

            if (node->tail != 0) {
            	// current node is the head, take next as head
            	m_dlConfigNodeHead = node->next;
				if (m_dlConfigNodeHead !=0 ) {
					m_dlConfigNodeHead->tail = node->tail;
				}
				prevNode = m_dlConfigNodeHead;
				m_nodePool->freeNode(node);
				node = m_dlConfigNodeHead;
            } else {
            	prevNode->next = node->next;
            	m_nodePool->freeNode(node);
            	node = prevNode->next;
            }
        } else {
        	LOG_DBG(UE_LOGGER_NAME, "[%s], skip the rest dl config nodes\n", __func__);
//            prevNode = node;
//            node = node->next;
            break;
        }
    }

    // --------------------------------------------------------
    // process DL Data
    node = m_dlDataNodeHead;
    prevNode = m_dlDataNodeHead;
    while (node != 0) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], node = %p\n", __func__, node);

        msgBuffer = (DlMsgBuffer*)node->buffer;
        UInt16 msgId = 0;
        FAPI_l1ApiMsg_st *pL1Api = (FAPI_l1ApiMsg_st *)msgBuffer->data;
        msgId = pL1Api->msgId;

        FapiL1MsgHead* pMsgHead = (FapiL1MsgHead*)&pL1Api->msgBody[0];
        UInt8 sf  = pMsgHead->sfnsf & 0x000f;
        UInt16 sfn  = (pMsgHead->sfnsf & 0xfff0) >> 4;
        LOG_DBG(UE_LOGGER_NAME, "[%s], handle PHY_DL_TX_REQUEST in %d.%d, the provision sfnsf is %d.%d\n", __func__,
            m_sfn, m_sf, sfn, sf);

        if (!validateSfnSf(FALSE, sfn, sf) || (sf == m_sf && sfn == m_sfn)) {
        	FAPI_dlDataTxRequest_st* pDlDataReq = (FAPI_dlDataTxRequest_st*)&pL1Api->msgBody[0];
			handleDlDataReq(pDlDataReq);

            // free the buffer and node
            msgBuffer->length = 0;

            if (node->tail != 0) {
            	// current node is the head, take next as head
            	m_dlDataNodeHead = node->next;
				if (m_dlDataNodeHead !=0 ) {
					m_dlDataNodeHead->tail = node->tail;
				}
				prevNode = m_dlDataNodeHead;
				m_nodePool->freeNode(node);
				node = m_dlDataNodeHead;
            } else {
            	prevNode->next = node->next;
            	m_nodePool->freeNode(node);
            	node = prevNode->next;
            }
        } else {
        	LOG_DBG(UE_LOGGER_NAME, "[%s], skip the rest dl data nodes\n", __func__);
            break;
        }
    }

    // --------------------------------------------------------
    // process HI/DCI0
    node = m_hiDci0NodeHead;
    prevNode = m_hiDci0NodeHead;
    while (node != 0) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], node = %p\n", __func__, node);

        msgBuffer = (DlMsgBuffer*)node->buffer;
        UInt16 msgId = 0;
        FAPI_l1ApiMsg_st *pL1Api = (FAPI_l1ApiMsg_st *)msgBuffer->data;
        msgId = pL1Api->msgId;

        FapiL1MsgHead* pMsgHead = (FapiL1MsgHead*)&pL1Api->msgBody[0];
        UInt8 sf  = pMsgHead->sfnsf & 0x000f;
        UInt16 sfn  = (pMsgHead->sfnsf & 0xfff0) >> 4;
        LOG_DBG(UE_LOGGER_NAME, "[%s], handle PHY_DL_HI_DCI0_REQUEST in %d.%d, the provision sfnsf is %d.%d\n", __func__,
            m_sfn, m_sf, sfn, sf);

        if (!validateSfnSf(FALSE, sfn, sf) || (sf == m_sf && sfn == m_sfn)) {
            FAPI_dlHiDCIPduInfo_st *pHIDci0Req = (FAPI_dlHiDCIPduInfo_st *)&pL1Api->msgBody[0];
            handleHIDci0Req(pHIDci0Req);

            // free the buffer and node
            msgBuffer->length = 0;

            if (node->tail != 0) {
            	// current node is the head, take next as head
            	m_hiDci0NodeHead = node->next;
				if (m_hiDci0NodeHead !=0 ) {
					m_hiDci0NodeHead->tail = node->tail;
				}
				prevNode = m_hiDci0NodeHead;
				m_nodePool->freeNode(node);
				node = m_hiDci0NodeHead;
            } else {
            	prevNode->next = node->next;
            	m_nodePool->freeNode(node);
            	node = prevNode->next;
            }
        } else {
        	LOG_DBG(UE_LOGGER_NAME, "[%s], skip the rest hi/dci0 nodes\n", __func__);
            break;
        }
    }

    // --------------------------------------------------------
    // process UE Config
    node = m_ueConfigNodeHead;
    prevNode = m_ueConfigNodeHead;
    while (node != 0) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], node = %p\n", __func__, node);

        msgBuffer = (DlMsgBuffer*)node->buffer;
        FAPI_l1ApiMsg_st *pL1Api = (FAPI_l1ApiMsg_st *)msgBuffer->data;

#ifndef OS_LINUX
        LOG_INFO(UE_LOGGER_NAME, "[%s], handle PHY_UE_CONFIG_REQUEST in %d.%d, node = %p\n", __func__, m_sfn, m_sf, node);
		FAPI_phyUeConfigRequest_st *pUeConfigReq = (FAPI_phyUeConfigRequest_st *)&pL1Api->msgBody[0];
		handleUeConfigReq(pUeConfigReq);

		// free the buffer and node
		msgBuffer->length = 0;
		m_ueConfigNodeHead = node->next;
		m_nodePool->freeNode(node);
		node = m_ueConfigNodeHead;
#else
        FapiL1MsgHead* pMsgHead = (FapiL1MsgHead*)&pL1Api->msgBody[0];
        UInt8 sf  = pMsgHead->sfnsf & 0x000f;
        UInt16 sfn  = (pMsgHead->sfnsf & 0xfff0) >> 4;
        LOG_INFO(UE_LOGGER_NAME, "[%s], handle PHY_DELETE_UE_REQUEST in %d.%d, provSfnSf = %d.%d\n", __func__, m_sfn, m_sf, sfn, sf);

        if (!validateSfnSf(FALSE, sfn, sf) || (sf == m_sf && sfn == m_sfn)) {
        	UInt16* pMsg = (UInt16*)&pL1Api->msgBody[0];
			UInt16* pRnti = ++pMsg;
			handleDeleteUeReq(*pRnti);

			// free the buffer and node
			msgBuffer->length = 0;

			if (node->tail != 0) {
				// current node is the head, take next as head
				m_ueConfigNodeHead = node->next;
				if (m_ueConfigNodeHead !=0 ) {
					m_ueConfigNodeHead->tail = node->tail;
				}
				prevNode = m_ueConfigNodeHead;
				m_nodePool->freeNode(node);
				node = m_ueConfigNodeHead;
			} else {
				prevNode->next = node->next;
				m_nodePool->freeNode(node);
				node = prevNode->next;
			}
		} else {
			LOG_DBG(UE_LOGGER_NAME, "[%s], skip the rest ue config nodes\n", __func__);
			break;
		}
#endif
    }

}

// --------------------------------------------------------
void UeScheduler::resetUeTerminal(UInt16 rnti, UInt8 ueId) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], rnti = %d, ueId = %d, %d.%d\n", __func__, rnti, ueId, m_sfn, m_sf);

    std::map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
    if (it != m_rntiUeIdMap.end()) {
        m_rntiUeIdMap.erase(it);
    }

    // should not happen
    it = m_rntiUeIdMap.begin();
    while (it != m_rntiUeIdMap.end()) {
    	if (it->second == ueId) {
    		LOG_WARN(UE_LOGGER_NAME, "[%s], multiple rnti exists, delete it, rnti = %d, ueId = %d\n", __func__, it->first, ueId);
    		m_rntiUeIdMap.erase(it++);
    	} else {
    		it++;
    	}
    }

    it = m_pduIndexUeIdMap.begin();
    while (it != m_pduIndexUeIdMap.end()) {
        if (it->second == ueId) {
            m_pduIndexUeIdMap.erase(it++);
        } else {
            it++;
        }
    }

    // it = m_harqIdUeIdMap.begin();
    // while (it != m_harqIdUeIdMap.end()) {
    //     if (it->second == ueId) {
    //         m_harqIdUeIdMap.erase(it++);
    //     } else {
    //         it++;
    //     }
    // }

    std::vector<UInt32>::iterator iter = m_ueIdHarqIdVect.begin();
    while (iter != m_ueIdHarqIdVect.end()) {
        UInt8 foundUeId = ((*iter) >> 16) & 0xff;
        if (foundUeId == ueId) {
            iter = m_ueIdHarqIdVect.erase(iter);
        } else {
            ++iter;
        }
    }
}

// ----------------------------------------
void UeScheduler::handleDlConfigReq(FAPI_dlConfigRequest_st* pDlConfigReq) {
    UInt32 headerLen = (UInt32)((UInt8*)&pDlConfigReq->dlConfigpduInfo[0] - (UInt8*)pDlConfigReq); 
    if((headerLen == pDlConfigReq->length)
        || ((pDlConfigReq->numDCI + pDlConfigReq->numOfPDU + pDlConfigReq->numOfPDSCHRNTI) <= 0))
    {
        LOG_WARN(UE_LOGGER_NAME, "[%s], Empty PHY_DL_CONFIG_REQUEST\n", __func__);
        return;
    } 

    UInt8 sf  = pDlConfigReq->sfnsf & 0x000f;
    UInt16 sfn  = (pDlConfigReq->sfnsf & 0xfff0) >> 4;

    LOG_TRACE(UE_LOGGER_NAME, "[%s], Recv PHY_DL_CONFIG_REQUEST, provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
        sfn, sf, m_sfn, m_sf);
    
    FAPI_dlConfigPDUInfo_st *pNextPdu, *pPrevPdu;
    pNextPdu = (FAPI_dlConfigPDUInfo_st *)&pDlConfigReq->dlConfigpduInfo[0];
    Int length = 0;
    length += ((uintptr_t)pNextPdu - (uintptr_t)pDlConfigReq);

    do{
        pPrevPdu = pNextPdu;

        LOG_DBG(UE_LOGGER_NAME, "[%s], Recv PHY_DL_CONFIG_REQUEST, pduType = %d\n", __func__, pNextPdu->pduType);

        switch (pNextPdu->pduType) {
            case FAPI_DCI_DL_PDU:
            {
                UInt16 rnti = pNextPdu->dlConfigpduInfo.DCIPdu.rnti;
                if (pNextPdu->dlConfigpduInfo.DCIPdu.dciFormat == FAPI_DL_DCI_FORMAT_1A ||
                    pNextPdu->dlConfigpduInfo.DCIPdu.dciFormat == FAPI_DL_DCI_FORMAT_1) {
                    if (rnti != 0xffff && rnti != 0xfffe) {
//                        LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_DL_DCI_FORMAT_1A / FAPI_DL_DCI_FORMAT_1 (%d), "
//                            "rnti = %d, curSfnSf = %d.%d\n", __func__,
//                            pNextPdu->dlConfigpduInfo.DCIPdu.dciFormat, rnti, m_sfn, m_sf);
                        
                        if (rnti <= m_maxRaRntiUeId) {
                            // For RAR, rnti is ra-rnti, which is the same as ueId
                            m_ueList[rnti-1]->handleDlDciPdu(pDlConfigReq, &pNextPdu->dlConfigpduInfo.DCIPdu);
                        } else {
                            // other msg, c-rnti
                            map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
                            if (it != m_rntiUeIdMap.end()) {
                                UInt8 ueId = it->second;
                                if (ueId <= m_maxRaRntiUeId) {
                                    m_ueList[ueId-1]->handleDlDciPdu(pDlConfigReq, &pNextPdu->dlConfigpduInfo.DCIPdu);
                                } else {
                                    LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
                                }
                            } else {
                                LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get ueId by rnti = %d\n", __func__, rnti);
                            }
                        }

                    } else {
                        LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_DL_DCI_FORMAT_1A (broadcast), "
                            "provSfnSf = %d.%d, rnti = %d, curSfnSf = %d.%d\n", __func__,
                            sfn, sf, rnti, m_sfn, m_sf);
                    }

                    pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize); 
                    if (pPrevPdu != pNextPdu) {
                        length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                    }
                } else {
                	LOG_INFO(UE_LOGGER_NAME, "[%s], provSfnSf = %d.%d, curSfnSf = %d.%d, rnti = %d, dciFormat = %d, TODO\n", __func__,
                            sfn, sf, m_sfn, m_sf, rnti, pNextPdu->dlConfigpduInfo.DCIPdu.dciFormat);

                    pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize); 
                    if (pPrevPdu != pNextPdu) {
                        length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                    }
                }
                break;
            }

            case FAPI_DLSCH_PDU:
            {
                UInt16 rnti = pNextPdu->dlConfigpduInfo.DlSCHPdu.rnti;
                UInt16 pduIndex = pNextPdu->dlConfigpduInfo.DlSCHPdu.pduIndex;
                if (rnti != 0xffff) {
                    LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_DLSCH_PDU, "
                        "provSfnSf = %d.%d, rnti = %d, curSfnSf = %d.%d\n", __func__,
                        sfn, sf, pNextPdu->dlConfigpduInfo.DlSCHPdu.rnti, m_sfn, m_sf);
                    // TODO further handling according to rnti
                    if (rnti <= m_maxRaRntiUeId) {
                        // For RAR, rnti is ra-rnti, which is the same as ueId
                        m_ueList[rnti-1]->handleDlSchPdu(pDlConfigReq, &pNextPdu->dlConfigpduInfo.DlSCHPdu);

                        // save the pduIndex as key to find ueId
                        pair<map<UInt16, UInt8>::iterator, bool> result = 
                            m_pduIndexUeIdMap.insert(map<UInt16, UInt8>::value_type(pduIndex, rnti));
                        if (!result.second) {
                            (result.first)->second = rnti;
                        }
                    } else {
                        // other msg, c-rnti
                        map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
                        if (it != m_rntiUeIdMap.end()) {
                            UInt8 ueId = it->second;
                            if (ueId <= m_maxRaRntiUeId) {
                                m_ueList[ueId-1]->handleDlSchPdu(pDlConfigReq, &pNextPdu->dlConfigpduInfo.DlSCHPdu);

                                // save the pduIndex as key to find ueId
                                pair<map<UInt16, UInt8>::iterator, bool> result = 
                                    m_pduIndexUeIdMap.insert(map<UInt16, UInt8>::value_type(pduIndex, ueId));
                                if (!result.second) {
                                    (result.first)->second = ueId;
                                }
                            } else {
                                LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
                            }
                        } else {
                            LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get ueId by rnti = %d\n", __func__, rnti);
                        }
                    }
                } else {
                    LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_DLSCH_PDU (broadcast), "
                        "provSfnSf = %d.%d, rnti = %d, curSfnSf = %d.%d\n", __func__,
                        sfn, sf, rnti, m_sfn, m_sf);
                }

                pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8*)pNextPdu) + pNextPdu->pduSize);
                if (pPrevPdu != pNextPdu) {
                    length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                }
                break;
            }

            case FAPI_PCH_PDU:
            case FAPI_PRS_PDU:
            {
                pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize);
                if (pPrevPdu != pNextPdu) {
                    length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                }
                break;
            }

            case FAPI_BCH_PDU:
            {
                // NOT handle MIB
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_BCH_PDU\n", __func__);
                pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize);
                if (pPrevPdu != pNextPdu) {
                    length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                }
                break;
            }

            default:
            	LOG_WARN(UE_LOGGER_NAME, "[%s], Invalid pduType = %d\n", __func__, pNextPdu->pduType);
                pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize);
                if (pPrevPdu != pNextPdu) {
                    length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                }
                break;
        }        

    } while(length < pDlConfigReq->length);
}

// ----------------------------------------
void UeScheduler::handleUlConfigReq(FAPI_ulConfigRequest_st* pUlConfigReq) {
    UInt32 headerLen = (UInt8*)&pUlConfigReq->ulPduConfigInfo[0] - (UInt8*)pUlConfigReq;

    if((headerLen == pUlConfigReq->ulConfigLen) || ((pUlConfigReq->numOfPdu) <= 0))
    {
        return;
    }

    UInt8 sf  = pUlConfigReq->sfnsf & 0x000f;
    UInt16 sfn  = (pUlConfigReq->sfnsf & 0xfff0) >> 4;

    LOG_TRACE(UE_LOGGER_NAME, "[%s], Recv PHY_UL_CONFIG_REQUEST, provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
        sfn, sf, m_sfn, m_sf);

    UInt8  *tmpBuff = (UInt8 *)(pUlConfigReq->ulPduConfigInfo);
    UInt16 pduSize = 0;
    UInt32 pduIndex=0;
    FAPI_ulPDUConfigInfo_st* ulPduConf_p = PNULL;
    for(; pduIndex < pUlConfigReq->numOfPdu; pduIndex++) {
        ulPduConf_p = (FAPI_ulPDUConfigInfo_st *)(tmpBuff + pduSize);

        switch(ulPduConf_p->ulConfigPduType) {                    
            case FAPI_ULSCH:
            {
                FAPI_ulSCHPduInfo_st *pUlSchPdu = (FAPI_ulSCHPduInfo_st *)(ulPduConf_p->ulPduConfigInfo);
                LOG_DBG(UE_LOGGER_NAME, "[%s], [%d.%d], Recv FAPI_ULSCH, rnti = %d, provSfnSf = %d.%d\n", __func__,
                    m_sfn, m_sf, pUlSchPdu->rnti, sfn, sf);

                map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(pUlSchPdu->rnti);
                if (it != m_rntiUeIdMap.end()) {
                    UInt8 ueId = it->second;
                    if (ueId <= m_maxRaRntiUeId) {
                        m_ueList[ueId-1]->handleUlSchPdu(pUlConfigReq, pUlSchPdu);
                    } else {
                        LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
                    }
                } else {
                    LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get ueId by rnti = %d\n", __func__, pUlSchPdu->rnti);
                }

                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_ULSCH_HARQ:
            {
                FAPI_ulSCHHarqPduInfo_st* ulschHarq_p = (FAPI_ulSCHHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_ULSCH_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    ulschHarq_p->ulSCHPduInfo.rnti, sfn, sf, m_sfn, m_sf);

                map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(ulschHarq_p->ulSCHPduInfo.rnti);
                if (it != m_rntiUeIdMap.end()) {
                    UInt8 ueId = it->second;
                    if (ueId <= m_maxRaRntiUeId) {
                        m_ueList[ueId-1]->handleUlSchPdu(pUlConfigReq, &ulschHarq_p->ulSCHPduInfo);
                    } else {
                        LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
                    }
                } else {
                    LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get ueId by rnti = %d\n", __func__, ulschHarq_p->ulSCHPduInfo.rnti);
                }

                // TODO count the ULSCH_HARQ msg 

                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_ULSCH_CQI_RI:
            {
                FAPI_ulSCHCqiRiPduInfo_st* ulschCqiRi_p = (FAPI_ulSCHCqiRiPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_ULSCH_CQI_RI, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    ulschCqiRi_p->ulSCHPduInfo.rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_ULSCH_CQI_HARQ_RI:
            {
                FAPI_ulSCHCqiHarqRIPduInfo_st* ulschCqiHarqRi_p = (FAPI_ulSCHCqiHarqRIPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_ULSCH_CQI_HARQ_RI, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    ulschCqiHarqRi_p->ulSCHPduInfo.rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_SR:
            {
                FAPI_uciSrPduInfo_st* uciSrPdu_p = (FAPI_uciSrPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_UCI_SR, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    uciSrPdu_p->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_HARQ:
            {
                FAPI_uciHarqPduInfo_st *uciHarq_p = (FAPI_uciHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_UCI_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    uciHarq_p->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_SR_HARQ:
            {
                FAPI_uciSrHarqPduInfo_st *uciSrHarq = (FAPI_uciSrHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_UCI_SR_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    uciSrHarq->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_CQI:
            {
                FAPI_uciCqiPduInfo_st *uciCqi = (FAPI_uciCqiPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_UCI_CQI, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    uciCqi->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_CQI_HARQ:
            {
                FAPI_uciCqiHarqPduInfo_st *uciCqiHarq = (FAPI_uciCqiHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_UCI_CQI_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    uciCqiHarq->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_CQI_SR:
            {
                FAPI_uciCqiSrPduInfo_st *uciCqiSr = (FAPI_uciCqiSrPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_UCI_CQI_SR, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    uciCqiSr->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_CQI_SR_HARQ:
            {
                FAPI_uciCqiSrHarqPduInfo_st *uciCqiSrHarq = (FAPI_uciCqiSrHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_UCI_CQI_SR_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    uciCqiSrHarq->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_SRS:
            {
                FAPI_srsPduInfo_st * srsPdu_p = (FAPI_srsPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_SRS, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    srsPdu_p->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }
        
            default:
            {
                LOG_WARN(UE_LOGGER_NAME, "[%s], Recv ulConfigPduType = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
                    ulPduConf_p->ulConfigPduType, sfn, sf, m_sfn, m_sf);
                break;
            }
        }
    }                  
}

// ----------------------------------------
void UeScheduler::handleHIDci0Req(FAPI_dlHiDCIPduInfo_st* pHIDci0Req) {
    // UInt32 headerLen = (UInt8*)&pHIDci0Req->hidciPduInfo[0] - (UInt8*)pHIDci0Req;
    if((pHIDci0Req->numOfDCI + pHIDci0Req->numOfHI) <= 0) {
        return;
    }

    UInt8 sf  = pHIDci0Req->sfnsf & 0x000f;
    UInt16 sfn  = (pHIDci0Req->sfnsf & 0xfff0) >> 4;
    LOG_TRACE(UE_LOGGER_NAME, "[%s], Recv PHY_DL_HI_DCI0_REQUEST, provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
        sfn, sf, m_sfn, m_sf);

    UInt8* pNextPdu = PNULL;
    UInt8* pPrevPdu = PNULL;
    
    pNextPdu = &(pHIDci0Req->hidciPduInfo[0]);

    UInt8 numOfDCI = 0;
    UInt8 numOfHI = 0;

    while((numOfDCI < pHIDci0Req->numOfDCI) || (numOfHI < pHIDci0Req->numOfHI)) {
        pPrevPdu = pNextPdu;
        UInt8 pduType = *((UInt8*)pNextPdu);
        if (pduType == FAPI_HI_PDU) { 
            // HARQ ACK for UL TB
            FAPI_dlHiPduInfo_st* pHiPdu = (FAPI_dlHiPduInfo_st*)pNextPdu;
            LOG_DBG(UE_LOGGER_NAME, "[%s], Recv FAPI_HI_PDU, rbStart = %d, cyclicShift2_forDMRS = %d, hiValue = %d, iPHICH = %d, txPower = %d\n", __func__,
                pHiPdu->rbStart, pHiPdu->cyclicShift2_forDMRS, pHiPdu->hiValue, pHiPdu->iPHICH, pHiPdu->txPower);

            UInt32 ueIdHarqId;
            UInt8 ueId;
            UInt16 harqId = (pHiPdu->rbStart << 8) | pHiPdu->cyclicShift2_forDMRS;
            vector<UInt32>::iterator it = m_ueIdHarqIdVect.begin();
            while(it!=m_ueIdHarqIdVect.end()) {
                ueIdHarqId = *it;
                LOG_DBG(UE_LOGGER_NAME, "[%s], ueIdHarqId = %04x\n", __func__, ueIdHarqId);
                if ((ueIdHarqId & 0xffff) == harqId) {
                    ueId = (ueIdHarqId >> 16) & 0xff;
                    if (ueId <= m_maxRaRntiUeId) {
                        if (m_ueList[ueId-1]->handleHIPdu(pHIDci0Req, pHiPdu)) {
                        	LOG_DBG(UE_LOGGER_NAME, "[%s], Handle harq ack success\n", __func__);
                            m_ueIdHarqIdVect.erase(it);
                            break;
                        } else {
                            LOG_ERROR(UE_LOGGER_NAME, "[%s], Handle harq ack failed in ueId = %d\n", __func__, ueId);
                        }
                    } else {
                        LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
                    }
                }

                ++it;
            }

            // UInt16 harqId = (pHiPdu->rbStart << 8) | pHiPdu->cyclicShift2_forDMRS;
            // map<UInt16, UInt8>::iterator it = m_harqIdUeIdMap.find(harqId);
            // if (it != m_harqIdUeIdMap.end()) {
            //     UInt8 ueId = it->second;
            //     if (ueId <= m_maxRaRntiUeId) {
            //         m_ueList[ueId-1]->handleHIPdu(pHIDci0Req, pHiPdu);

            //         // TBD. really need to delete it?
            //         m_harqIdUeIdMap.erase(it);
            //     } else {
            //         LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
            //     }
            // } else {
            //     LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get ueId by harqId = %d\n", __func__, harqId);
            // }

            numOfHI++;
            pNextPdu = ((UInt8 *)pNextPdu) + sizeof(FAPI_dlHiPduInfo_st);
 
        } else if (pduType == FAPI_DCI_UL_PDU) {
            FAPI_dlDCIPduInfo_st* pDciPdu = (FAPI_dlDCIPduInfo_st*)pNextPdu;
            UInt16 rnti = pDciPdu->rnti;          
            map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
            if (it != m_rntiUeIdMap.end()) {
                UInt8 ueId = it->second;
                if (ueId <= m_maxRaRntiUeId) {
                    m_ueList[ueId-1]->handleDci0Pdu(pHIDci0Req, pDciPdu);

                    UInt32 ueIdHarqId = (ueId << 16) | (pDciPdu->rbStart << 8) | pDciPdu->cyclicShift2_forDMRS;
                    m_ueIdHarqIdVect.push_back(ueIdHarqId);

                    // // save the harqId as key to find ueId
                    // UInt16 harqId = (pDciPdu->rbStart << 8) | pDciPdu->cyclicShift2_forDMRS;
                    // pair<map<UInt16, UInt8>::iterator, bool> result = 
                    //     m_harqIdUeIdMap.insert(map<UInt16, UInt8>::value_type(harqId, ueId));
                    // if (!result.second) {
                    //     LOG_WARN(UE_LOGGER_NAME, "[%s], harqId 0x%04x record exists, overide it\n", __func__, harqId);
                    //     (result.first)->second = ueId;
                    // }
                } else {
                    LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
                }
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "[%s], Fail to get ueId by rnti = %d\n", __func__, rnti);
            }

            numOfDCI++;
            pNextPdu = ((UInt8 *)pNextPdu) + pDciPdu->ulDCIPDUSize;
        } else {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid pduType = %d\n", __func__, pduType);
            break;
        }
    }
}

// ----------------------------------------
void UeScheduler::handleDlDataReq(FAPI_dlDataTxRequest_st* pDlDataReq) {

    UInt8 sf  = pDlDataReq->sfnsf & 0x000f;
    UInt16 sfn  = (pDlDataReq->sfnsf & 0xfff0) >> 4;

    if (pDlDataReq->numOfPDU == 0) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], no data pdu received\n", __func__);
        return;
    }

    LOG_TRACE(UE_LOGGER_NAME, "[%s], Recv PHY_DL_TX_REQUEST, provSfnSf = %d.%d, curSfnSf = %d.%d\n", __func__,
        sfn, sf, m_sfn, m_sf);
    
    FAPI_dlTLVInfo_st *pDlTlv = PNULL;
    FAPI_dlPduInfo_st *pNextPdu = PNULL;
    FAPI_dlPduInfo_st *pPrevPdu = PNULL;
    pNextPdu = (FAPI_dlPduInfo_st *)&pDlDataReq->dlPduInfo[0];
    Int length = 0;
    length += ((uintptr_t)pNextPdu - (uintptr_t)pDlDataReq);

    Int i;
    for(i = 0; i < pDlDataReq->numOfPDU; i++) {
        pPrevPdu = pNextPdu;
        if (pNextPdu->numOfTLV != 1) {
            LOG_WARN(UE_LOGGER_NAME, "[%s], Invalid numOfTLV = %d\n", __func__, pNextPdu->numOfTLV);
            break;
        }
        pDlTlv = (FAPI_dlTLVInfo_st *)&pNextPdu->dlTLVInfo[0];
        if (pDlTlv->tag != 0) {
            LOG_WARN(UE_LOGGER_NAME, "[%s], Invalid tag = %d\n", __func__, pDlTlv->tag);
            break;
        }

        UInt16 pduIndex = pNextPdu->pduIndex;
        map<UInt16, UInt8>::iterator it = m_pduIndexUeIdMap.find(pduIndex);
        if (it != m_pduIndexUeIdMap.end()) {
            UInt8 ueId = it->second;
            if (ueId <= m_maxRaRntiUeId) {
                m_ueList[ueId-1]->handleDlTxData(pDlDataReq, pDlTlv, this);
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "[%s], Invalid ueId = %d\n", __func__, ueId);
            }
        } else {
            LOG_DBG(UE_LOGGER_NAME, "[%s], Fail to get ueId by pduIndex = %d, it could be BCH pdu\n", __func__, pduIndex);
        }

        pNextPdu = (FAPI_dlPduInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduLen);
    }
    
}
