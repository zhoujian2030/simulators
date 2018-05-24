/*
 * UeTerminal.cpp
 *
 *  Created on: Nov 05, 2016
 *      Author: j.zhou
 */

#include <stddef.h>
#include <stdio.h>
#include "UeTerminal.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif
#include "PhyMacAPI.h"
#include "UeScheduler.h"
#include "StsCounter.h"
#include "HarqEntity.h"
#include "RlcLayer.h"
#include "PdcpLayer.h"
#include <vector>

using namespace ue;
using namespace std;

const UInt8 UeTerminal::m_ulSubframeList[10] = {0, 0, 1, 0, 0, 0, 0, 1, 0, 0};
// --------------------------------------------
UeTerminal::UeTerminal(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: m_suspend(FALSE), m_accessCount(0), m_maxAccessCount(0), m_phyMacAPI(phyMacAPI), m_ueId(ueId), m_stsCounter(stsCounter),
  m_raRnti(raRnti), m_preamble(preamble), m_ta(31),m_rachTa(0), m_state(IDLE),
  m_subState(IDLE), m_rachSf(SUBFRAME_SENT_RACH), m_rachSfn(0), m_srConfigIndex(17),
  m_dsrTransMax(64), m_srCounter(0)
{
	LOG_TRACE(UE_LOGGER_NAME, "[%s], Entry\n", __func__);
    m_harqEntity = new HarqEntity(stsCounter, NUM_UL_HARQ_PROCESS, NUM_DL_HARQ_PROCESS);

    m_t300Value = -1;
    m_contResolutionTValue = -1;
    m_srTValue = -1;
    m_srPeriodicity = 0;
    m_bsrTValue = -1;
    m_needSendSR = FALSE;
    m_dlSchMsg = new DlSchMsg();
    m_msg3.randomValue = 0;
    m_msg3.cause = mo;
    m_msg3.lcCCCH = lc_ccch;
    m_msg3.lcPadding = lc_padding;

    m_rrcLayer = new RrcLayer(this);
    m_pdcpLayer = new PdcpLayer(m_rrcLayer);
    m_rlcLayer = new RlcLayer(this, m_pdcpLayer);
    m_triggerIdRsp = FALSE;
    m_triggerRlcStatusPdu = FALSE;

#ifdef OS_LINUX
    m_rachSfnDelay = (m_ueId - 1) / 2 + 2;
    m_maxRachIntervalSfn = 0; 
#else
    m_rachSfnDelay = ((m_ueId - 1) / 2 + 250) % 1024;
    m_maxRachIntervalSfn = 5; // 10 sfn = 100ms
#endif
    m_firstRachSent = FALSE;
    m_firstRachSfnSet = FALSE;

    m_rachIntervalSfnCnt = 0;

    m_biRecvd = FALSE;

    sprintf(m_uniqueId, "[%02x.%04x.%04x]", m_ueId, m_raRnti, 0xffff);
    //LOG_DBG(UE_LOGGER_NAME, "[%s], %s\n",  __func__, m_uniqueId);
}

// ------------------------------------------------------
void UeTerminal::reset() {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s\n",  __func__, m_uniqueId);
    m_harqEntity->reset();
    m_rlcLayer->reset();
    m_pdcpLayer->reset();
    m_rrcLayer->reset();
    m_needSendSR = FALSE;
    m_triggerIdRsp = FALSE;
    m_triggerRlcStatusPdu = FALSE;
    m_t300Value = -1;
    m_contResolutionTValue = -1;
    m_srTValue = -1;
    m_bsrTValue = -1;
    m_state = IDLE;

    m_suspend = FALSE;

    m_rachIntervalSfnCnt = 0;

    resetChild();
}

// ------------------------------------------------------
void UeTerminal::resetChild() {

}

// --------------------------------------------
UeTerminal::~UeTerminal() {
	delete m_harqEntity;
}

// --------------------------------------------
void UeTerminal::updateConfig(UInt32 maxAccessCount) {
	if (m_accessCount >= m_maxAccessCount && m_state == IDLE) {
		m_firstRachSent = FALSE;
		m_firstRachSfnSet = FALSE;
		m_rachIntervalSfnCnt = 0;
	}

	m_maxAccessCount = maxAccessCount;
	m_accessCount = 0;
}

// --------------------------------------------
void UeTerminal::showConfig() {
	LOG_INFO(UE_LOGGER_NAME, "[%s], %s, m_maxAccessCount = %d, m_accessCount = %d\n",  __func__, m_uniqueId, m_maxAccessCount, m_accessCount);

}

// --------------------------------------------
BOOL UeTerminal::schedule(UInt16 sfn, UInt8 sf, UeScheduler* pUeScheduler) {
    sprintf(&m_uniqueId[14], "-[%04d.%d]", sfn, sf);
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s\n",  __func__, m_uniqueId);
    
    m_sfn = sfn;
    m_sf = sf;   

#if 0
    if (m_biRecvd) {
    	static UInt16 cnt = 0;
    	cnt++;
    	if (cnt > 1000) {
    		cnt = 0;
    		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, receive BI in MSG2, stop scheduling.\n",  __func__, m_uniqueId);
    	}
    	return FALSE;
    }

    if (isT300Expired()) {
        this->reset();
        m_stsCounter->countT300Timeout();
        m_stsCounter->countTestFailure();
        pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, T300 expired, reset state to IDLE\n",  __func__, m_uniqueId);
    }

    if (!this->scheduleRach(pUeScheduler)) {
    	return FALSE;
    }
    this->scheduleMsg3(pUeScheduler);
    this->scheduleSR(pUeScheduler);
    this->scheduleDCCH(pUeScheduler);
    this->processDlHarq(pUeScheduler);
    this->processTimer(pUeScheduler);
    // TODO
#else
    if (!this->scheduleDcchOnly(pUeScheduler)) {
    	return FALSE;
    }
#endif

    return TRUE;
}

// --------------------------------------------
void UeTerminal::handleCreateUeReq(UInt16 srConfigIndex) {
	LOG_INFO(UE_LOGGER_NAME, "[%s], %s, m_state = %d, srConfigIndex = %d\n",  __func__, m_uniqueId, m_state, srConfigIndex);

	m_srConfigIndex = srConfigIndex;
    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getUeConfigMsgBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UE_CONFIG_RESPONSE;
    pL1Api->msgLen = 0;
    m_phyMacAPI->addUeConfigMsgLength(FAPI_HEADER_LENGTH);
    m_phyMacAPI->sendUeConfigResp();
}

// --------------------------------------------
void UeTerminal::handleDeleteUeReq() {
	LOG_INFO(UE_LOGGER_NAME, "[%s], %s, m_state = %d\n",  __func__, m_uniqueId, m_state);

    if (m_state == RRC_RELEASING) {
        m_stsCounter->countTestSuccess();
    } else {
    	m_stsCounter->countInvalidState();
        m_stsCounter->countTestFailure();
    }
    m_state = WAIT_TERMINATING;

    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getUeConfigMsgBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UE_CONFIG_RESPONSE;
    pL1Api->msgLen = 0;
    m_phyMacAPI->addUeConfigMsgLength(FAPI_HEADER_LENGTH);
    m_phyMacAPI->sendUeConfigResp();
}

// --------------------------------------------
BOOL UeTerminal::scheduleRach(UeScheduler* pUeScheduler) {
	if (m_accessCount >= m_maxAccessCount && m_state == IDLE) {
		return FALSE;
	}

	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d\n",  __func__, m_uniqueId, m_state);
#if 0
	if (m_state == IDLE && m_sf == m_rachSf) {
		// delay some ms after previous access
		if (m_rachIntervalSfnCnt >= m_maxRachIntervalSfn) {
			m_rachSfn = m_sfn;
		} else {
			m_rachIntervalSfnCnt++;
			return TRUE;
		}

		m_rnti = 0;
		m_raTicks = 0;
		sprintf(&m_uniqueId[9], "%04x]-[%04d.%d]", 0xffff, m_sfn, m_sf);

		FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getRachBuffer();
		FAPI_rachIndication_st* pRachInd = (FAPI_rachIndication_st *)&pL1Api->msgBody[0];
		if (pRachInd->numOfPreamble >= 3) {
			LOG_INFO(UE_LOGGER_NAME, "[%s], %s, will not send rach in %d.%d, as numOfPreamble is already %d\n",  __func__,
					m_uniqueId, m_rachSfn, m_rachSf, pRachInd->numOfPreamble);
			return TRUE;
		}

		m_accessCount++;
		if (m_accessCount > m_maxAccessCount) {
			LOG_INFO(UE_LOGGER_NAME, "[%s], %s, reach m_maxAccessCount = %d, return\n",  __func__, m_maxAccessCount, m_uniqueId);
			return FALSE;
		}

		m_rachIntervalSfnCnt = 0;

		UInt32 msgLen = 0;
		pL1Api->lenVendorSpecific = 0;
		pL1Api->msgId = PHY_UL_RACH_INDICATION;
		pRachInd->sfnsf = ( (m_rachSfn) << 4) | ( (m_rachSf) & 0xf);
		pRachInd->numOfPreamble += 1;
		UInt32 rachHeaderLen = offsetof(FAPI_rachIndication_st, rachPduInfo);

		FAPI_rachPduIndication_st* pRachPduInd = (FAPI_rachPduIndication_st*)&pRachInd->rachPduInfo[pRachInd->numOfPreamble-1];
		pRachPduInd->rnti = m_raRnti;
		pRachPduInd->preamble = m_preamble;
		pRachPduInd->timingAdvance = m_rachTa;  //TODO

		msgLen += sizeof(FAPI_rachPduIndication_st);
		pL1Api->msgLen += msgLen;

		m_phyMacAPI->addRachDataLength(msgLen);
		if (pRachInd->numOfPreamble == 1) {
			m_phyMacAPI->addRachDataLength(FAPI_HEADER_LENGTH + rachHeaderLen);
			pL1Api->msgLen += rachHeaderLen;
		}

		this->startT300();
		m_state = MSG1_SENT;

		m_stsCounter->countRachSent();

		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose rach indication, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
	} else {
        // check RACH timer
        if (m_state == MSG1_SENT || m_state == MSG2_DCI_RECVD || m_state == MSG2_SCH_RECVD) {
            m_raTicks++;
            if (m_raTicks <= (subframeDelayAfterRachSent + raResponseWindowSize)) {
                return TRUE;
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, RACH timeout\n",  __func__, m_uniqueId);
                m_stsCounter->countRachTimeout();
                this->reset();
                m_stsCounter->countTestFailure();
                pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
            }
        }
    }

#else
    if (!m_firstRachSfnSet) {
        m_rachSfn = (m_sfn + m_rachSfnDelay) % 1024;
        m_firstRachSfnSet = TRUE;
        LOG_INFO(UE_LOGGER_NAME, "[%s], %s, will send first rach in %d.%d\n",  __func__, m_uniqueId, m_rachSfn, m_rachSf);
    }

    if (m_state == IDLE && m_sf == m_rachSf) {
        if (!m_firstRachSent) {
            if (m_sfn == m_rachSfn) {
                m_firstRachSent = TRUE;
                m_rachSfn = m_sfn;
            } else {
                return TRUE;
            }          
        } else {
        	// delay some ms after previous access
        	if (m_rachIntervalSfnCnt >= m_maxRachIntervalSfn) {
                m_rachSfn = m_sfn;
                m_rachIntervalSfnCnt = 0;
        	} else {
        		m_rachIntervalSfnCnt++;
        		return TRUE;
        	}
        }

        m_accessCount++;

    	if (m_accessCount > m_maxAccessCount) {
    		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, reach m_maxAccessCount = %d, return\n",  __func__, m_maxAccessCount, m_uniqueId);
    		return FALSE;
    	}

        m_raTicks = 0;

        // reset m_rnti value
        m_rnti = 0; 
        sprintf(&m_uniqueId[9], "%04x]-[%04d.%d]", 0xffff, m_sfn, m_sf);

        // only send rach in specisl subframe 1 according SIB2
        UInt32 msgLen = 0;
        FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getRachBuffer();
        pL1Api->lenVendorSpecific = 0;
        pL1Api->msgId = PHY_UL_RACH_INDICATION;

        FAPI_rachIndication_st* pRachInd = (FAPI_rachIndication_st *)&pL1Api->msgBody[0];
        pRachInd->sfnsf = ( (m_rachSfn) << 4) | ( (m_rachSf) & 0xf);        
        pRachInd->numOfPreamble += 1;
        UInt32 rachHeaderLen = offsetof(FAPI_rachIndication_st, rachPduInfo);

        FAPI_rachPduIndication_st* pRachPduInd = (FAPI_rachPduIndication_st*)&pRachInd->rachPduInfo[pRachInd->numOfPreamble-1];
        pRachPduInd->rnti = m_raRnti;
        pRachPduInd->preamble = m_preamble;
        pRachPduInd->timingAdvance = m_rachTa;  //TODO
        
        msgLen += sizeof(FAPI_rachPduIndication_st);
        pL1Api->msgLen += msgLen;

        m_phyMacAPI->addRachDataLength(msgLen);
        if (pRachInd->numOfPreamble == 1) {
            m_phyMacAPI->addRachDataLength(FAPI_HEADER_LENGTH + rachHeaderLen);
            pL1Api->msgLen += rachHeaderLen;
        }

        this->startT300();
        m_state = MSG1_SENT;

        m_stsCounter->countRachSent();

        LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose rach indication, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
    } else {
        // check RACH timer
        if (m_state == MSG1_SENT || m_state == MSG2_DCI_RECVD || m_state == MSG2_SCH_RECVD) {
            m_raTicks++;
            if (m_raTicks <= (subframeDelayAfterRachSent + raResponseWindowSize)) {
                return TRUE;
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, RACH timeout\n",  __func__, m_uniqueId);
                m_stsCounter->countRachTimeout();
                this->reset();
                m_stsCounter->countTestFailure();
                pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
            }
        }
    }
#endif

    return TRUE;
}

// --------------------------------------------
void UeTerminal::scheduleMsg3(UeScheduler* pUeScheduler) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s\n",  __func__, m_uniqueId);
    if (m_state == MSG2_RECVD) {
        // to send MSG3 in the UL subframe
        if (m_sfn == m_msg3Sfn && m_sf == m_msg3Sf) {
            buildMsg3Data();
            // buildMsg3WithRnti();
            buildCrcData(0);
            m_stsCounter->countMsg3CrcSent();
            m_state = MSG3_SENT;
            startContentionResolutionTimer(); 
        } else {
            // TODO exception in case frame lost
        }
    } else if (m_state == MSG3_SENT) {
        if (processContentionResolutionTimer()) {
            this->reset();
            m_stsCounter->countTestFailure();
            pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
        }
    } else {
        // TODO
    }
}

// --------------------------------------------
void UeTerminal::scheduleSR(UeScheduler* pUeScheduler) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s\n",  __func__, m_uniqueId);

    if (m_needSendSR) {
        if (m_sfn == m_srSfn && m_sf == m_srSf) {
            UInt32 msgLen = 0;
            FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getSrBuffer();
            pL1Api->lenVendorSpecific = 0;
            pL1Api->msgId = PHY_UL_RX_SR_INDICATION;

            FAPI_rxSRIndication_st *pSrInd = (FAPI_rxSRIndication_st*)&pL1Api->msgBody[0];
            pSrInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
            pSrInd->numOfSr += 1;
            UInt32 srHeaderLen = offsetof(FAPI_rxSRIndication_st, srPduInfo);

            FAPI_srPduIndication_st *pSrPduInd = (FAPI_srPduIndication_st *)&pSrInd->srPduInfo[pSrInd->numOfSr - 1];
            pSrPduInd->rnti = m_rnti;

            msgLen += sizeof(FAPI_srPduIndication_st);
            pL1Api->msgLen += msgLen;

            m_phyMacAPI->addSrDataLength(msgLen);
            if (pSrInd->numOfSr == 1) {
                m_phyMacAPI->addSrDataLength(FAPI_HEADER_LENGTH + srHeaderLen);
                pL1Api->msgLen += srHeaderLen;
            }

            if (m_state == RRC_SETUP_ACK_SENT) {
                m_state = RRC_SETUP_COMPLETE_SR_SENT;
            } else {

            }

            startSRTimer();
            m_needSendSR = FALSE;

            m_stsCounter->countSRSent();

            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose SR, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
        } else {
            // TODO exception in case frame lost
        }
    } else {
        if (processSRTimer()) {
            if (m_srCounter < m_dsrTransMax) {
            	LOG_WARN(UE_LOGGER_NAME, "[%s], %s, prepare retransmitting SR, m_srCounter = %d\n",  __func__, m_uniqueId, m_srCounter);
                setSfnSfForSR(TRUE);
            } else {
                this->reset();
                m_stsCounter->countTestFailure();
                pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
            }
        }
    }
}

// ---------------------------------------------------
#if 1
#define  DL_CONFIG_REQUEST                      (0x80)
#define  UL_CONFIG_REQUEST                      (0x81)
#define  SUBFRAME_INDICATION                    (0x82)
#define  UL_DCI_REQUEST                         (0x83)
#define  TX_REQUEST                             (0x84)
#define  HARQ_INDICATION                        (0x85)
#define  RX_ULSCH_INDICATION                    (0x87)
#define  RACH_INDICATION                        (0x88)
#define  SRS_INDICATION                         (0x89)
#define  RX_SR_INDICATION                       (0x8a)
#define  RX_CQI_INDICATION                      (0x8b)
#define  RX_ULCRC_INDICATION                    (0x8c)
#define  MSG_INVALID                            (0xFF)

typedef struct  PhyHlMsgHead
{
	UInt32 mNum;
	UInt32 tLen;
	UInt32 sno;
	UInt8  attr;
	UInt8  common;
	UInt16 opc;

	UInt32 UeId;
	UInt16 cRnti;
	UInt8  cellId;
	UInt8  reserved1;
	UInt32 handle;
	UInt32 reserved2;
}S_PhyHlMsgHead;

typedef struct UlIndHead
{
	UInt16    sfn;
	UInt8     sf;
	UInt8     numOfPDUs;

}S_UlIndHead;

typedef struct RxUlschIndHeadPdu
{
	UInt32  UeId;

	UInt16   RNTI;
	UInt8    RNTIType;
	UInt8    mcs;

	UInt32   bitLen;

	UInt16   wordLen;
	UInt8    CRCFlag;
	Int8     SNR;

	Int16    TA;
	UInt8    rbNum;
	UInt8    Reserved;

	Int32	prbPower;
	Int32	puschRssi;

    // UInt32  PDU[Length]

}S_RxUlschIndHeadPdu;

typedef struct {
	UInt16 rnti;
	UInt16 length;
	UInt8* buffer;
} UlSchPdu;
// --------------------------------------------
BOOL UeTerminal::scheduleDcchOnly(UeScheduler* pUeScheduler)
{
	if (m_accessCount >= m_maxAccessCount && m_state == IDLE) {
		return FALSE;
	}

	unsigned short length = 0;
	UInt8* pMsgBuffer;
	S_PhyHlMsgHead* pPhyMsgHead;
	S_UlIndHead* pUlIndHead;
	S_RxUlschIndHeadPdu* pUlSchPduHead;

	if (m_sf == 2 || m_sf == 7) {
		if (m_state == IDLE) {
			UInt8 macPdu[RRC_SETUP_COMPLETE_LENGTH] = {
				0x3D, 0x21, 0x4b, 0x1F, 0x00, 0xA0, 0x00, 0x00,  // need RLC status report
				0x22, 0x00, 0x82, 0x0E, 0x82, 0xE2, 0x10, 0x92, 0x0C, 0x00, 0x2A, 0x69, 0x04, 0xC2, 0x4C, 0x09,
				0xC1, 0xC1, 0x81, 0x80, 0x00, 0x46, 0x04, 0x03, 0xA0, 0x62, 0x4E, 0x3B, 0x01, 0x00, 0x42, 0x20,
				0x02, 0x02, 0x00, 0x21, 0x02, 0x0C, 0x00, 0x00, 0x00, 0x01, 0x06, 0x0C, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x14, 0x00, 0xB8, 0x40, 0x00, 0x62, 0x07, 0x2A, 0xC0,
				0x28, 0xBA, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00, 0x54, 0x03, 0xEB, 0x3F, 0x55};

			pMsgBuffer = m_phyMacAPI->getSchBuffer();
			pPhyMsgHead = (S_PhyHlMsgHead*)pMsgBuffer;
			pPhyMsgHead->opc = RX_ULSCH_INDICATION;

			pUlIndHead = (S_UlIndHead*)(pMsgBuffer + sizeof(S_PhyHlMsgHead));
			pUlIndHead->sfn = m_sfn;
			pUlIndHead->sf = m_sf;
			pUlIndHead->numOfPDUs += 1;

			if (pUlIndHead->numOfPDUs == 1) {
				length += sizeof(S_PhyHlMsgHead);
				length += sizeof(S_UlIndHead);
				pUlSchPduHead = (S_RxUlschIndHeadPdu*)(pMsgBuffer + length);
			} else {
				pUlSchPduHead = (S_RxUlschIndHeadPdu*)(pMsgBuffer + m_phyMacAPI->getSchDataLength());
			}
			length += sizeof(S_RxUlschIndHeadPdu);
			m_phyMacAPI->addSchDataLength(length);

			pUlSchPduHead->RNTI = m_raRnti + 100;
			pUlSchPduHead->CRCFlag = 1;
			pUlSchPduHead->wordLen = (RRC_SETUP_COMPLETE_LENGTH + 3) >> 2;
			pUlSchPduHead->bitLen = RRC_SETUP_COMPLETE_LENGTH << 3;
//			m_phyMacAPI->addSchPduData(macPdu, RRC_SETUP_COMPLETE_LENGTH);
			memcpy(pMsgBuffer + m_phyMacAPI->getSchDataLength(), macPdu, RRC_SETUP_COMPLETE_LENGTH);
			m_phyMacAPI->addSchDataLength((pUlSchPduHead->wordLen << 2));

			m_state = RRC_SETUP_COMPLETE_SENT;

			m_stsCounter->countRRCSetupComplSent();

			LOG_INFO(UE_LOGGER_NAME, "[%s], %s, send rrc setup complete, length = %d\n",  __func__, m_uniqueId, m_phyMacAPI->getSchDataLength());

		} else if (m_state == RRC_SETUP_COMPLETE_SENT) {
			UInt8 macPdu[] = {
					0x3D, 0x21, 0x02, 0x21, 0x15, 0x1F, 0x00, 0x00, 0x04, 0xA0,
					0x01, 0x01, 0x48, 0x01, 0x60, 0xEA, 0xC1, 0x09, 0x20, 0xC8,
					0x02, 0x26, 0x80, 0xF2, 0x4E, 0x80, 0x00, 0x00, 0x00, 0x00,
					0x04, 0x03, 0xA0, 0x62, 0x4E, 0x3B, 0x01, 0x00, 0x42, 0x20,
					0x02, 0x02, 0x00, 0x21, 0x02, 0x0C, 0x00, 0x00, 0x00, 0x01,
					0x06, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
					0x06, 0x00, 0x00, 0x14, 0x00, 0xB8, 0x40, 0x00, 0xBA, 0x02,
					0x08, 0x00, 0x00, 0x00, 0x00, 0x3B, 0x8E, 0x8A, 0x41, 0x2B,
					0xF3, 0x53, 0x24, 0x5A, 0x1F};

			pMsgBuffer = m_phyMacAPI->getSchBuffer();
			pPhyMsgHead = (S_PhyHlMsgHead*)pMsgBuffer;
			pPhyMsgHead->opc = RX_ULSCH_INDICATION;

			pUlIndHead = (S_UlIndHead*)(pMsgBuffer + sizeof(S_PhyHlMsgHead));
			pUlIndHead->sfn = m_sfn;
			pUlIndHead->sf = m_sf;
			pUlIndHead->numOfPDUs += 1;

			if (pUlIndHead->numOfPDUs == 1) {
				length += sizeof(S_PhyHlMsgHead);
				length += sizeof(S_UlIndHead);
				pUlSchPduHead = (S_RxUlschIndHeadPdu*)(pMsgBuffer + length);
			} else {
				pUlSchPduHead = (S_RxUlschIndHeadPdu*)(pMsgBuffer + m_phyMacAPI->getSchDataLength());
			}
			length += sizeof(S_RxUlschIndHeadPdu);
			m_phyMacAPI->addSchDataLength(length);

			pUlSchPduHead->RNTI = m_raRnti + 100;
			pUlSchPduHead->CRCFlag = 1;
			pUlSchPduHead->wordLen = (sizeof(macPdu) + 3) >> 2;
			pUlSchPduHead->bitLen = sizeof(macPdu) << 3;
//			m_phyMacAPI->addSchPduData(macPdu, sizeof(macPdu));
			memcpy(pMsgBuffer + m_phyMacAPI->getSchDataLength(), macPdu, sizeof(macPdu));
			m_phyMacAPI->addSchDataLength((pUlSchPduHead->wordLen << 2));

			m_state = IDLE;

			m_accessCount++;

			m_stsCounter->countIdentityResponseSent();

			LOG_INFO(UE_LOGGER_NAME, "[%s], %s, send identity response, length = %d\n",  __func__, m_uniqueId, m_phyMacAPI->getSchDataLength());
		}
	}

	return TRUE;
}
#endif

// --------------------------------------------
void UeTerminal::scheduleDCCH(UeScheduler* pUeScheduler) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d\n",  __func__, m_uniqueId, m_state);

    if (processBSRTimer()) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, BSR timeout, terminate connection\n",  __func__, m_uniqueId);
        m_stsCounter->countTestFailure();
        m_state = WAIT_TERMINATING;
    }

    switch (m_state) {
        case RRC_SETUP_COMPLETE_DCI0_RECVD:
        case RRC_SETUP_COMPLETE_SR_DCI0_RECVD:
        {
            m_harqEntity->send(this);
            break;
        }

        case RRC_SETUP_COMPLETE_SENT:
        {
            m_harqEntity->send(this);
        }
        case RRC_SETUP_COMPLETE_BSR_SENT:
        {
            m_harqEntity->calcAndProcessUlHarqTimer(this);
            break;
        }

        case WAIT_TERMINATING:
        {
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, terminate the connection\n",  __func__, m_uniqueId);
            this->reset();
            pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
            break;
        }

        default:
        {
            if ((m_state >= RRC_SETUP_COMPLETE_SENT) && (m_state <= RRC_CONNECTED)) {
            	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle ul harq in state = %d\n",  __func__, m_uniqueId, m_state);
                m_harqEntity->send(this);
                m_harqEntity->calcAndProcessUlHarqTimer(this);
            }

            if (m_state == RRC_RELEASING) {
            	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle ul harq in state = %d\n",  __func__, m_uniqueId, m_state);

                m_harqEntity->send(this);
                m_harqEntity->calcAndProcessUlHarqTimer(this);
            }
            
            break;
        }
    }
}

// --------------------------------------------
void UeTerminal::processDlHarq(UeScheduler* pUeScheduler) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s\n",  __func__, m_uniqueId);
    m_harqEntity->sendAck(m_sfn, m_sf, this);
}

// --------------------------------------------
void UeTerminal::processTimer(UeScheduler* pUeScheduler) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s\n",  __func__, m_uniqueId);
    if (m_state == MSG4_ACK_SENT) {
        if (processRRCSetupTimer()) {
            this->reset();
            m_stsCounter->countTestFailure();
            pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
        }
    }

    if (m_state > RRC_SETUP_COMPLETE_SR_SENT) {
    	if (m_rlcLayer->processTimer()) {
    		m_stsCounter->countRlcTimeout();
    		m_state = WAIT_TERMINATING;
    	}
    }
}

// --------------------------------------------------------
BOOL UeTerminal::processContentionResolutionTimer() {
    if (m_contResolutionTValue < 0) {
        return FALSE;
    }

    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_contResolutionTValue = %d\n",  __func__, m_uniqueId, m_contResolutionTValue);

    if (m_contResolutionTValue == 0) {
        // contention resolution timeout
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, No Contention Resolution received\n",  __func__, m_uniqueId);
        m_stsCounter->countContentionResolutionTimeout();
        m_contResolutionTValue = -1;
        return TRUE;
    } 
        
    m_contResolutionTValue--;
    return FALSE;
}

// --------------------------------------------------------
BOOL UeTerminal::processRRCSetupTimer() {
    if (m_rrcSetupTValue < 0) {
        return FALSE;
    }

    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_rrcSetupTValue = %d\n",  __func__, m_uniqueId, m_rrcSetupTValue);

    if (m_rrcSetupTValue == 0) {
        // wait RRC Setup timeout
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, No RRC Setup received\n",  __func__, m_uniqueId);
        m_stsCounter->countRRCSetupTimeout();
        m_rrcSetupTValue = -1;
        return TRUE;
    } 
        
    m_rrcSetupTValue--;
    return FALSE;    
}

// -------------------------------------------------------
BOOL UeTerminal::processSRTimer() {
    if (m_srTValue < 0) {
        return FALSE;
    }

    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_srTValue = %d\n", __func__, m_uniqueId, m_srTValue);
    
    if (m_srTValue == 0) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, SR timeout\n",  __func__, m_uniqueId);
        m_stsCounter->countSRTimeout();
        m_srTValue = -1;
        return TRUE;
    }

    m_srTValue--;
    return FALSE;
}

// -------------------------------------------------------
BOOL UeTerminal::processBSRTimer() {
    if (m_bsrTValue < 0) {
        return FALSE;
    }

    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_bsrTValue = %d\n",  __func__, m_uniqueId, m_bsrTValue);
    
    if (m_bsrTValue == 0) {
        m_bsrTValue = -1;
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, BSR timeout\n",  __func__, m_uniqueId);
        m_stsCounter->countBSRTimeout();
        return TRUE;
    }

    m_bsrTValue--;
    return FALSE;
}

// --------------------------------------------
void UeTerminal::buildCrcData(UInt8 crcFlag) {
    UInt32 msgLen = 0;

    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getCrcBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UL_CRC_INDICATION;

    FAPI_crcIndication_st *pCrcInd = (FAPI_crcIndication_st*)&pL1Api->msgBody[0];
    pCrcInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
    pCrcInd->numOfCrc += 1;
    UInt32 crcHeaderLen = offsetof(FAPI_crcIndication_st, crcPduInfo);

    FAPI_crcPduIndication_st *pCrcPduInd = (FAPI_crcPduIndication_st *)&pCrcInd->crcPduInfo[pCrcInd->numOfCrc-1];
    pCrcPduInd->rnti = m_rnti; 
    pCrcPduInd->crcFlag = crcFlag; // 0 : CRC correct

    msgLen += sizeof(FAPI_crcPduIndication_st);
    pL1Api->msgLen += msgLen;

    m_phyMacAPI->addCrcDataLength(msgLen);
    if (pCrcInd->numOfCrc == 1) {
        m_phyMacAPI->addCrcDataLength(FAPI_HEADER_LENGTH + crcHeaderLen);
        pL1Api->msgLen += crcHeaderLen;
    }       

    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, compose crc indication, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
}

// --------------------------------------------
void UeTerminal::buildMsg3WithRnti() {
    UInt32 msgLen = 0;
    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getSchBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UL_RX_ULSCH_INDICATION;

    FAPI_rxULSCHIndication_st *pULSchInd = (FAPI_rxULSCHIndication_st*)&pL1Api->msgBody[0];
    pULSchInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
    pULSchInd->numOfPdu += 1;

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
    pUlDataPduInd->ulCqi = 160;   
    pUlDataPduInd->timingAdvance = m_ta;
    pUlDataPduInd->dataOffset = 1;  // TBD

    msgLen += sizeof(FAPI_ulDataPduIndication_st);

    UInt8 msg3Buffer[MSG3_LENGTH] = {
        0x3B, 0x3D, 0x01, 0x00, 0x4B, 0x0E, 0x88, 0x02, 0x02, 0x48, 
        0x09, 0x40, 0xE9, 0x0E, 0x41, 0x7E, 0xCC, 0x9E, 0x00, 0x14,
        0xA0, 0xFA};

    msgLen += pUlDataPduInd->length;
    pL1Api->msgLen += msgLen;
    m_phyMacAPI->addSchDataLength(msgLen);

    m_phyMacAPI->addSchPduData(msg3Buffer, pUlDataPduInd->length);

    m_stsCounter->countMsg3Sent();

    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose MSG3 with C-RNTI, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
}

// --------------------------------------------
void UeTerminal::buildMsg3Data() {
    m_msg3.randomValue++;

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
    pUlDataPduInd->ulCqi = 160;   //TBD
    pUlDataPduInd->timingAdvance = m_ta;
    pUlDataPduInd->dataOffset = 1;  // TBD

    msgLen += sizeof(FAPI_ulDataPduIndication_st);

    // randomValue = (ueId << 32) | m_msg3.randomValue
    UInt8 msg3Buffer[MSG3_LENGTH];
    UInt32 i = 0;
    msg3Buffer[i++] = 0x20 | m_msg3.lcCCCH;
    msg3Buffer[i++] = 6;
    msg3Buffer[i++] = m_msg3.lcPadding;
    msg3Buffer[i++] = 0x50 | ((m_ueId & 0xf0) >> 4);
    msg3Buffer[i++] = ((m_ueId & 0x0f) << 4) | ((m_msg3.randomValue & 0xf0000000) >> 28);
    msg3Buffer[i++] = ((m_msg3.randomValue & 0x0f000000) >> 20) | ((m_msg3.randomValue & 0x00f00000) >> 20);
    msg3Buffer[i++] = ((m_msg3.randomValue & 0x000f0000) >> 12) | ((m_msg3.randomValue & 0x0000f000) >> 12);
    msg3Buffer[i++] = ((m_msg3.randomValue & 0x00000f00) >> 4 ) | ((m_msg3.randomValue & 0x000000f0) >> 4);
    msg3Buffer[i++] = ((m_msg3.randomValue & 0x0000000f) << 4 ) | (m_msg3.cause << 1);

    msgLen += pUlDataPduInd->length;
    pL1Api->msgLen += msgLen;
    m_phyMacAPI->addSchDataLength(msgLen);

    m_phyMacAPI->addSchPduData(msg3Buffer, pUlDataPduInd->length);

    m_stsCounter->countMsg3Sent();

    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose MSG3 (RRC Connection Connection Request), msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
}

#define MIN_UL_TB_LENGTH 18 // 128
// --------------------------------------------
void UeTerminal::buildBSRAndData(BOOL isLongBSR) {
    UInt32 msgLen = 0;
    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_phyMacAPI->getSchBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UL_RX_ULSCH_INDICATION;

    FAPI_rxULSCHIndication_st *pULSchInd = (FAPI_rxULSCHIndication_st*)&pL1Api->msgBody[0];
    pULSchInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
    pULSchInd->numOfPdu += 1;

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
    pUlDataPduInd->length = BSR_MSG_LENGTH;
    pUlDataPduInd->ulCqi = 160;   //TBD 0 ~ 255
    pUlDataPduInd->timingAdvance = m_ta;
    pUlDataPduInd->dataOffset = 1;  // TBD

    msgLen += sizeof(FAPI_ulDataPduIndication_st);  

    if (!isLongBSR) {
        if (!m_triggerRlcStatusPdu) {
            if (m_triggerIdRsp) {
                UInt8 bsr[BSR_MSG_LENGTH] = {0x3a, 0x3d, 0x1f, 0x00, 0x08};
                m_phyMacAPI->addSchPduData(bsr, pUlDataPduInd->length);
            } else {
                // for rrc setup complete
                UInt8 bsr[BSR_MSG_LENGTH] = {0x3a, 0x3d, 0x1f, 0x00, 0x0e};
                m_phyMacAPI->addSchPduData(bsr, pUlDataPduInd->length);               
            }
        } else {
            if (m_triggerIdRsp) {
                UInt8 bsr[BSR_MSG_LENGTH] = {0x3d, 0x21, 0x02, 0x1f, 0x08, m_rlcStatusPdu[0], m_rlcStatusPdu[1]};
                m_phyMacAPI->addSchPduData(bsr, pUlDataPduInd->length);
            } else {
                UInt8 bsr[BSR_MSG_LENGTH] = {0x3d, 0x21, 0x02, 0x1f, 0x0e, m_rlcStatusPdu[0], m_rlcStatusPdu[1]};
                m_phyMacAPI->addSchPduData(bsr, pUlDataPduInd->length);               
            }
            m_triggerRlcStatusPdu = FALSE;
        }
        LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose BSR to request UL resource, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
        startBSRTimer();
    } else {
        if (!m_triggerIdRsp && !m_triggerRlcStatusPdu) {
            UInt8 longBsr[BSR_MSG_LENGTH] = {0x3E, 0x1F, 0x00, 0x00, 0x00};
            m_phyMacAPI->addSchPduData(longBsr, pUlDataPduInd->length);
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose zero long BSR\n",  __func__, m_uniqueId);
        } else {
            if (m_triggerIdRsp) {
            	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, add Identity Response\n",  __func__, m_uniqueId);
                m_triggerIdRsp = FALSE;
                UInt8 identityRsp[IDENTITY_MSG_LENGTH];
                UInt32 length = 0;
                m_rrcLayer->buildIdentityResponse(identityRsp, length);
                m_pdcpLayer->buildSrb1Header(identityRsp, length);   
                m_rlcLayer->buildRlcAMDHeader(identityRsp, length); 

                m_stsCounter->countIdentityResponseSent();

                if (m_triggerRlcStatusPdu) {
                	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, add RlC status PDU\n",  __func__, m_uniqueId);
                    m_triggerRlcStatusPdu = FALSE;
                    UInt8 macPdu[IDENTITY_MSG_LENGTH] = {0x3d, 0x21, 0x02, 0x21, 0x80,(UInt8)length, 0x1f, 0x00, m_rlcStatusPdu[0], m_rlcStatusPdu[1]};
                    memcpy(macPdu + 10, identityRsp, length);
                    pUlDataPduInd->length = IDENTITY_MSG_LENGTH;
                    m_phyMacAPI->addSchPduData(macPdu, pUlDataPduInd->length);
                    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose BSR and identity response and RLC ACK, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
                } else {
                    UInt8 macPdu[IDENTITY_MSG_LENGTH] = {0x3D, 0x21, 0x80, (UInt8)length, 0x1F, 0x00};                    
                    memcpy(macPdu + 6, identityRsp, length);
                    pUlDataPduInd->length = IDENTITY_MSG_LENGTH;
                    m_phyMacAPI->addSchPduData(macPdu, pUlDataPduInd->length);
                    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose BSR and identity response, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
                }

                // for test
                //m_state =  WAIT_TERMINATING;
                //LOG_INFO(UE_LOGGER_NAME, "[%s], Release UE resource now!!!!!!!!!!!!!\n", __func__);
            } else {
                LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, add RlC status PDU\n",  __func__, m_uniqueId);
                m_triggerRlcStatusPdu = FALSE;
                UInt8 macPdu[MIN_UL_TB_LENGTH] = {0x3d, 0x21, 0x02, 0x1f, 0x00, m_rlcStatusPdu[0], m_rlcStatusPdu[1]};
                pUlDataPduInd->length = MIN_UL_TB_LENGTH;
                m_phyMacAPI->addSchPduData(macPdu, pUlDataPduInd->length);
                LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose BSR and RLC ACK, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
            }
        }
    }

    msgLen += pUlDataPduInd->length;
    pL1Api->msgLen += msgLen;
    m_phyMacAPI->addSchDataLength(msgLen);
}

// --------------------------------------------
void UeTerminal::buildRRCSetupComplete() {    
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
    pUlDataPduInd->length = RRC_SETUP_COMPLETE_LENGTH;
    pUlDataPduInd->ulCqi = 160;   //TBD 0 ~ 255
    pUlDataPduInd->timingAdvance = m_ta;
    pUlDataPduInd->dataOffset = 1;  // TBD

    msgLen += sizeof(FAPI_ulDataPduIndication_st);

    // ------------------------------------------------------------
    // mac&rlc header
    // 3a 3d 21 48 1f 00 00 a0 00 00
    // rrc setup complete
    // 22 20 03 62 cc 3b 17 62 16 0f 
    // 34 02 07 48 01 0b f6 64 f0 00 
    // 03 62 cc d0 92 9b d9 58 04 e0 
    // e0 c0 c0 52 64 f0 00 25 03 5c 
    // 20 00 57 02 20 00 31 03 95 60 
    // 34 13 64 f0 00 25 03 11 03 57 
    // 58 96 5d 01 04
    // UInt8 rrcSetupCompl[RRC_SETUP_COMPLETE_LENGTH] = { 
    //     0x3A, 0x3D, 0x21, 0x48, 0x1F, 0x00, 0x00, 0xA0, 0x00, 0x00,
    //     0x22, 0x20, 0x03, 0x62, 0xCC, 0x3B, 0x17, 0x62, 0x16, 0x0F, 
    //     0x34, 0x02, 0x07, 0x48, 0x01, 0x0B, 0xF6, 0x64, 0xF0, 0x00, 
    //     0x03, 0x62, 0xCC, 0xD0, 0x92, 0x9B, 0xD9, 0x58, 0x04, 0xE0, 
    //     0xE0, 0xC0, 0xC0, 0x52, 0x64, 0xF0, 0x00, 0x25, 0x03, 0x5C, 
    //     0x20, 0x00, 0x57, 0x02, 0x20, 0x00, 0x31, 0x03, 0x95, 0x60, 
    //     0x34, 0x13, 0x64, 0xF0, 0x00, 0x25, 0x03, 0x11, 0x03, 0x57, 
    //     0x58, 0x96, 0x5D, 0x01, 0x04 };
    // -------------------------------------------------------------
    // or 
    // mac&rlc header
    // 3A 3D 21 80 88 1F 0A 00 A0 00 00 
    // rrc setup complete
    // 22 30 03 62 D2 7A 17 EB E5 0E 
    // DA 08 07 41 12 0B F6 64 F0 00 
    // 03 62 D2 E0 9B 9B 5C 05 F0 70 
    // 00 40 19 00 29 02 2E D0 31 27 
    // 23 80 80 21 10 01 00 00 10 81 
    // 06 00 00 00 00 83 06 00 00 00 
    // 00 00 0D 00 00 03 00 00 0A 00 
    // 00 05 00 00 10 00 52 64 F0 00 
    // 25 03 5C 0A 00 31 03 E5 E0 2E 
    // 13 64 F0 00 25 03 11 03 57 58 
    // A6 20 0A 60 14 04 62 91 03 00 
    // 12 1E 00 40 08 04 02 60 04 00 
    // 02 1F 00 5D 01 00 E0 C1 60
    UInt8 rrcSetupCompl[RRC_SETUP_COMPLETE_LENGTH] = { 
#if 1
        0x3D, 0x21, 0x4b, 0x1F, 0x00, 0xA0, 0x00, 0x00,  // need RLC status report
#else
        0x3D, 0x21, 0x4b, 0x1F, 0x00, 0x80, 0x00, 0x00,  // no need RLC status report
#endif

#if 0
        0x22, 0x30, 0x03, 0x62, 0xD2, 0x7A, 0x17, 0xEB, 0xE5, 0x0E, 
        0xDA, 0x08, 0x07, 0x41, 0x12, 0x0B, 0xF6, 0x64, 0xF0, 0x00, 
        0x03, 0x62, 0xD2, 0xE0, 0x9B, 0x9B, 0x5C, 0x05, 0xF0, 0x70, 
        0x00, 0x40, 0x19, 0x00, 0x29, 0x02, 0x2E, 0xD0, 0x31, 0x27, 
        0x23, 0x80, 0x80, 0x21, 0x10, 0x01, 0x00, 0x00, 0x10, 0x81, 
        0x06, 0x00, 0x00, 0x00, 0x00, 0x83, 0x06, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x0D, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0A, 0x00, 
        0x00, 0x05, 0x00, 0x00, 0x10, 0x00, 0x52, 0x64, 0xF0, 0x00, 
        0x25, 0x03, 0x5C, 0x0A, 0x00, 0x31, 0x03, 0xE5, 0xE0, 0x2E, 
        0x13, 0x64, 0xF0, 0x00, 0x25, 0x03, 0x11, 0x03, 0x57, 0x58, 
        0xA6, 0x20, 0x0A, 0x60, 0x14, 0x04, 0x62, 0x91, 0x03, 0x00, 
        0x12, 0x1E, 0x00, 0x40, 0x08, 0x04, 0x02, 0x60, 0x04, 0x00, 
        0x02, 0x1F, 0x00, 0x5D, 0x01, 0x00, 0xE0, 0xC1, 0x60};
#else 
        0x22, 0x00, 0x82, 0x0E, 0x82, 0xE2, 0x10, 0x92, 0x0C, 0x00, 0x2A, 0x69, 0x04, 0xC2, 0x4C, 0x09,
        0xC1, 0xC1, 0x81, 0x80, 0x00, 0x46, 0x04, 0x03, 0xA0, 0x62, 0x4E, 0x3B, 0x01, 0x00, 0x42, 0x20,
        0x02, 0x02, 0x00, 0x21, 0x02, 0x0C, 0x00, 0x00, 0x00, 0x01, 0x06, 0x0C, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x14, 0x00, 0xB8, 0x40, 0x00, 0x62, 0x07, 0x2A, 0xC0,
        0x28, 0xBA, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00, 0x54, 0x03, 0xEB, 0x3F, 0x55};
#endif

    msgLen += pUlDataPduInd->length;
    pL1Api->msgLen += msgLen;
    m_phyMacAPI->addSchDataLength(msgLen);

    m_phyMacAPI->addSchPduData(rrcSetupCompl, pUlDataPduInd->length);

    m_stsCounter->countRRCSetupComplSent();

    m_rlcLayer->startTimer();

    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, compose RRC setup complete, msgLen = %d\n",  __func__, m_uniqueId, pL1Api->msgLen);
}

// --------------------------------------------
void UeTerminal::handleDlDciPdu(FAPI_dlConfigRequest_st* pDlConfigHeader, FAPI_dciDLPduInfo_st* pDlDciPdu) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle FAPI_dciDLPduInfo_st, m_state = %d\n",  __func__, m_uniqueId, m_state);
    
    if (m_state == IDLE || m_state == WAIT_TERMINATING) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, UE in idle or terminating state, drop the data\n",  __func__, m_uniqueId);
        return;
    }

    if (m_suspend) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, UE suspended\n",  __func__, m_uniqueId);
    	return;
    }

    //displayDciPduInfo(pDlDciPdu);
    
    UInt8 sf  = pDlConfigHeader->sfnsf & 0x000f;
    UInt16 sfn  = (pDlConfigHeader->sfnsf & 0xfff0) >> 4;

    if (sfn != m_sfn || sf != m_sf) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, global tick may not consecutive, terminate connection, provSfnSf = %d.%d\n",
        		__func__, m_uniqueId, sfn, sf);
        m_stsCounter->countNonConsecutiveSfnSf();
        m_stsCounter->countTestFailure();
        m_state = WAIT_TERMINATING;
        return;
    }

    UInt16 rnti = pDlDciPdu->rnti;
    if (rnti == m_raRnti) {
        // recv RAR DCI PDU
        // if (validateDlSfnSf(sfn, sf)) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, recv RAR DCI PDU\n",  __func__, m_uniqueId);
		m_state = MSG2_DCI_RECVD;
        // } else {
        //     LOG_WARN(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], invalid time config for RAR DCI pdu\n",  __func__, 
        //         m_ueId, m_raRnti);
        // }
    } else {
        // recv other DL DCI PDU
        // if (validateDlSfnSf(sfn, sf)) {
		m_harqEntity->allocateDlHarqProcess(pDlConfigHeader->sfnsf, pDlDciPdu, this);
        // } else {
        //     LOG_WARN(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], [C-RNTI: %d], invalid time config for DL DCI pdu\n",  __func__, 
        //         m_ueId, m_raRnti, rnti);
        // }
    }
}

// --------------------------------------------
void UeTerminal::displayDciPduInfo(FAPI_dciDLPduInfo_st* pDlDciPdu) {
    switch (pDlDciPdu->dciFormat) {
        case FAPI_DL_DCI_FORMAT_1:
        {
            FAPI_dciFormat1_st *pDciMsg = (FAPI_dciFormat1_st *)&pDlDciPdu->dciPdu[0];
            LOG_TRACE(UE_LOGGER_NAME, "[%s], [UE: %d], FAPI_DL_DCI_FORMAT_1, aggregationLevel = %d, mcs_1 = %d, tpc = %d, txPower = %d\n",
                __func__, m_ueId, pDciMsg->aggregationLevel, pDciMsg->mcs_1, pDciMsg->tpc, pDciMsg->txPower);
            break;
        }

        case FAPI_DL_DCI_FORMAT_1A:
        {
            FAPI_dciFormat1A_st *pDciMsg = (FAPI_dciFormat1A_st *)&pDlDciPdu->dciPdu[0];
            LOG_TRACE(UE_LOGGER_NAME, "[%s], [UE: %d], FAPI_DL_DCI_FORMAT_1A, aggregationLevel = %d, mcs_1 = %d, tpc = %d, txPower = %d\n",
                __func__, m_ueId, pDciMsg->aggregationLevel, pDciMsg->mcs_1, pDciMsg->tpc, pDciMsg->txPower);
            break;
        }

        default:
        {
        	LOG_TRACE(UE_LOGGER_NAME, "[%s], [UE: %d], other dciFormat = %d\n",  __func__, m_ueId, pDlDciPdu->dciFormat);
        }
    }
}

// --------------------------------------------
void UeTerminal::handleDlSchPdu(FAPI_dlConfigRequest_st* pDlConfigHeader, FAPI_dlSCHConfigPDUInfo_st* pDlSchPdu) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle FAPI_dlSCHConfigPDUInfo_st, m_state = %d\n",  __func__, m_uniqueId, m_state);

    if (m_state == IDLE || m_state == WAIT_TERMINATING) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, UE in idle or terminating state, drop the data\n",  __func__, m_uniqueId);
        return;
    }  

    if (m_suspend) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, UE suspended\n",  __func__, m_uniqueId);
    	return;
    }

    m_provSf  = pDlConfigHeader->sfnsf & 0x000f;
    m_provSfn  = (pDlConfigHeader->sfnsf & 0xfff0) >> 4;

    if (m_provSfn != m_sfn || m_provSf != m_sf) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, global tick may not consecutive, terminate connection, provSfnSf = %d.%d\n",
        		__func__, m_uniqueId, m_provSfn, m_provSf);
        m_stsCounter->countNonConsecutiveSfnSf();
        m_stsCounter->countTestFailure();
        m_state = WAIT_TERMINATING;
        return;
    }

    UInt16 rnti = pDlSchPdu->rnti;
    if (rnti == m_raRnti) {
        // recv RAR SCH PDU
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, provSfnSf = %d.%d\n",  __func__, m_uniqueId, m_provSfn, m_provSf);
        // if (validateDlSfnSf(m_provSfn, m_provSf)) {
            if (m_state == MSG2_DCI_RECVD ) {
                m_state = MSG2_SCH_RECVD;
                LOG_DBG(UE_LOGGER_NAME, "[%s], %s, recv RAR SCH PDU\n",  __func__, m_uniqueId);
            } else {
                LOG_WARN(UE_LOGGER_NAME, "[%s], %s, invalid RAR SCH pdu in m_state = %d\n",  __func__, m_uniqueId, m_state);
            }
        // } else {
        //     LOG_WARN(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], invalid time config for RAR SCH pdu\n",  __func__, 
        //         m_ueId, m_raRnti);
        // }
    } else {
        // recv other SCH PDU
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, mcs = %d\n",  __func__, m_uniqueId, pDlSchPdu->mcs);
        m_harqEntity->handleDlSchConfig(pDlConfigHeader->sfnsf, pDlSchPdu, this);
        // if (validateDlSfnSf(m_provSfn, m_provSf)) {
        //     LOG_DBG(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], [C-RNTI: %d], rbCoding = %d, transmissionScheme = %d\n",  __func__, 
        //         m_ueId, m_raRnti, rnti, pDlSchPdu->rbCoding, pDlSchPdu->transmissionScheme);
        //     if (m_state == MSG4_DCI_RECVD ) {
        //         m_state = MSG4_SCH_RECVD;
        //         LOG_DBG(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv Contention resolution SCH PDU\n",  __func__, m_ueId, m_raRnti, rnti);
        //     } else if (m_state == RRC_SETUP_DCI_RECVD) {
        //         m_state = RRC_SETUP_SCH_RECVD;
        //         LOG_DBG(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv RRC setup SCH PDU\n",  __func__, m_ueId, m_raRnti, rnti);
        //     } else {
        //         // TODO
        //         LOG_DBG(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv other SCH PDU, not implemented\n",  __func__, m_ueId, m_raRnti, rnti);
        //     }
        // } else {
        //     LOG_WARN(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], invalid time config for other SCH pdu\n",  __func__, 
        //         m_ueId, m_raRnti);
        // }
    } 
}

// --------------------------------------------
void UeTerminal::handleDlTxData(FAPI_dlDataTxRequest_st* pDlDataTxHeader, FAPI_dlTLVInfo_st *pDlTlv, UeScheduler* pUeScheduler) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle FAPI_dlTLVInfo_st, length = %d, m_state = %d\n",  __func__,
        m_uniqueId, pDlTlv->tagLen, m_state); 
    
    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, UE in idle state, drop the data\n",  __func__, m_uniqueId);
        return;
    } 

    if (m_suspend) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, UE suspended\n",  __func__, m_uniqueId);
    	return;
    }

    UInt8 sf  = pDlDataTxHeader->sfnsf & 0x000f;
    UInt16 sfn  = (pDlDataTxHeader->sfnsf & 0xfff0) >> 4;
    if (sfn != m_sfn || sf != m_sf) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, global tick may not consecutive, terminate connection, provSfnSf = %d.%d\n",
        		__func__, m_uniqueId, sfn, sf);
        m_stsCounter->countNonConsecutiveSfnSf();
        m_stsCounter->countTestFailure();
        m_state = WAIT_TERMINATING;
        return;
    }    

    UInt32 byteLen = pDlTlv->tagLen;
    UInt8* data = (UInt8*)pDlTlv->value;
    LOG_BUFFER(data, byteLen);

    if (m_state == MSG2_SCH_RECVD) {
        UInt16 sfnsf = (m_provSfn << 4) | m_provSf;
        if (sfnsf != pDlDataTxHeader->sfnsf) {
            LOG_WARN(UE_LOGGER_NAME, "[%s], %s, Invalid sfnsf = 0x%04x\n",  __func__, m_uniqueId, pDlDataTxHeader->sfnsf);
            return;
        }

        if (parseRarPdu(data, byteLen)) {
            m_state = MSG2_RECVD;
            this->setSfnSfForMsg3();
            this->m_rnti = m_dlSchMsg->rar.tcRnti;
            sprintf(&m_uniqueId[9], "%04x]-[%04d.%d]", m_rnti, m_sfn, m_sf);
            pUeScheduler->updateRntiUeIdMap(m_rnti, m_ueId);
            m_stsCounter->countRarRecvd();
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv RAR msg and change m_state to %d, MSG3 will be sent in %d.%d\n",
                __func__, m_uniqueId, m_state, m_msg3Sfn, m_msg3Sf); 
        } else {
            m_stsCounter->countRarInvalid();
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Fail to parse RAR\n",  __func__, m_uniqueId);
        }   
    } else {
        m_harqEntity->receive(pDlDataTxHeader->sfnsf, data, byteLen, this);
    }
}

// --------------------------------------------
void UeTerminal::handleUlSchPdu(FAPI_ulConfigRequest_st* pUlConfigHeader, FAPI_ulSCHPduInfo_st* pUlSchPdu) {
    if (m_suspend) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, UE suspended\n",  __func__, m_uniqueId);
    	return;
    }

    UInt16 sfn = (pUlConfigHeader->sfnsf & 0xfff0) >> 4;
    UInt8 sf = pUlConfigHeader->sfnsf & 0x0f;

    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, configure to receive UL SCH data in tick %d.%d\n",
        __func__, m_uniqueId, sfn, sf); 
    
    if (sfn != m_sfn || sf != m_sf) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, global tick may not consecutive, terminate connection, provSfnSf = %d.%d\n",
            __func__, m_uniqueId, sfn, sf);
        m_stsCounter->countNonConsecutiveSfnSf();
        m_stsCounter->countTestFailure();
        m_state = WAIT_TERMINATING;
        return;
    }

    // LOG_DBG(UE_LOGGER_NAME, "[%s], numOfRB = %d, cyclicShift2forDMRS = %d\n",  __func__, pUlSchPdu->numOfRB, pUlSchPdu->cyclicShift2forDMRS);
    displayUlSchPduInfo(pUlSchPdu);
    
    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, UE in idle state, drop the data\n",  __func__, m_uniqueId);
        return;
    } 

    if (m_state == MSG2_RECVD || m_state == MSG3_SENT) {
        if (sfn != m_msg3Sfn || sf != m_msg3Sf) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid frame configured for MSG3\n",  __func__, m_uniqueId);
            return;
        }        
        // only count this msg for statistics, UE will send MSG3 no matter MAC 
        // send this UL config or not
        m_stsCounter->countMsg3ULCfgRecvd();
        LOG_INFO(UE_LOGGER_NAME, "[%s], %s, receive UL config to receive MSG3\n",  __func__, m_uniqueId);
    } else {
        m_harqEntity->handleUlSchConfig(pUlConfigHeader->sfnsf, (void*)pUlSchPdu, this);
    }
}

// --------------------------------------------
void UeTerminal::displayUlSchPduInfo(FAPI_ulSCHPduInfo_st* pUlSchPdu) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s\n",  __func__, m_uniqueId);

	LOG_TRACE(UE_LOGGER_NAME, "[%s], rbStart = %d, numOfRB = %d, cyclicShift2forDMRS = %d, freqHoppingenabledFlag = %d\n",  __func__,
        pUlSchPdu->rbStart, pUlSchPdu->numOfRB, pUlSchPdu->cyclicShift2forDMRS, pUlSchPdu->freqHoppingenabledFlag);
	LOG_TRACE(UE_LOGGER_NAME, "[%s], freqHoppingBits = %d, harqProcessNumber = %d, currentTxNB = %d, modulationType = %d\n",  __func__,
        pUlSchPdu->freqHoppingBits, pUlSchPdu->harqProcessNumber, pUlSchPdu->currentTxNB, pUlSchPdu->modulationType);
    
}

// --------------------------------------------
void UeTerminal::handleDci0Pdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlDCIPduInfo_st* pDci0Pdu) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle DCI0, m_state = %d\n",  __func__, m_uniqueId, m_state);

    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, UE in idle state, drop the data\n",  __func__, m_uniqueId);
        return;
    }

    if (m_suspend) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, UE suspended\n",  __func__, m_uniqueId);
    	return;
    }

    UInt16 sfn = (pHIDci0Header->sfnsf & 0xfff0) >> 4;
    UInt8 sf = pHIDci0Header->sfnsf & 0x0f;
    if (sfn != m_sfn || sf != m_sf) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, global tick may not consecutive, terminate connection, provSfnSf = %d.%d\n",
        		__func__, m_uniqueId, sfn, sf);
        m_stsCounter->countNonConsecutiveSfnSf();
        m_stsCounter->countTestFailure();
        m_state = WAIT_TERMINATING;
        return;
    }

    // UInt8 sf  = pHIDci0Header->sfnsf & 0x000f;
    // UInt16 sfn  = (pHIDci0Header->sfnsf & 0xfff0) >> 4;
    UInt8 dciFormat = pDci0Pdu->ulDCIFormat;
    // UInt16 harqId = (pDci0Pdu->rbStart << 8) | pDci0Pdu->cyclicShift2_forDMRS;

    if (dciFormat == FAPI_UL_DCI_FORMAT_0) {
        UInt8 numOfRB = pDci0Pdu->numOfRB;
        LOG_INFO(UE_LOGGER_NAME, "[%s], %s, receive FAPI_UL_DCI_FORMAT_0, rbStart = %d, "
            "cyclicShift2_forDMRS = %d, numOfRB = %d\n",  __func__, m_uniqueId, pDci0Pdu->rbStart, 
            pDci0Pdu->cyclicShift2_forDMRS, numOfRB); 

        // eNb allocates the UL resource, stop the timer and prepare to send RRC setup complete            
        if (m_state == RRC_SETUP_COMPLETE_SR_SENT) {                 
            m_harqEntity->allocateUlHarqProcess(pHIDci0Header, pDci0Pdu, this);
            // stop the SR timer even it fails to allocate harq process for sending UL data
            stopSRTimer();                
            // m_stsCounter->countRRCSetupComplDCI0Recvd();
            
        } else if (m_state == RRC_SETUP_COMPLETE_BSR_ACK_RECVD) {  
            m_harqEntity->allocateUlHarqProcess(pHIDci0Header, pDci0Pdu, this);       
            stopBSRTimer();
            m_stsCounter->countRRCSetupComplDCI0Recvd();
        } else {
            if (isSRSent()) {
            	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, Recv UL Grant for SR, state = %d\n",  __func__, m_uniqueId, m_state);
                stopSRTimer();        
            } else if (isNonZeroBSRSent()) {
            	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, Recv UL Grant for BSR, state = %d\n",  __func__, m_uniqueId, m_state);
                stopBSRTimer(); 
            } else {
            	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, Recv force UL Grant, TBD state = %d\n",  __func__, m_uniqueId, m_state);
                m_stsCounter->countForceULGrantRecvd();
            }

            // TODO
            m_harqEntity->allocateUlHarqProcess(pHIDci0Header, pDci0Pdu, this); 
        }
    } else {
    	LOG_INFO(UE_LOGGER_NAME, "[%s], %s, handle DCI0, unsupported dciFormat = %d\n",  __func__, m_uniqueId, dciFormat);
    }
}

// --------------------------------------------
BOOL UeTerminal::handleHIPdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlHiPduInfo_st* pHiPdu) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle HI, m_state = %d\n",  __func__, m_uniqueId, m_state);

    if (m_suspend) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, UE suspended\n",  __func__, m_uniqueId);
    	return TRUE;
    }

    UInt16 sfn = (pHIDci0Header->sfnsf & 0xfff0) >> 4;
    UInt8 sf = pHIDci0Header->sfnsf & 0x0f;
    if (sfn != m_sfn || sf != m_sf) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, global tick may not consecutive, terminate connection, provSfnSf = %d.%d\n",
        		__func__, m_uniqueId, sfn, sf);
        m_stsCounter->countNonConsecutiveSfnSf();
        m_stsCounter->countTestFailure();
        m_state = WAIT_TERMINATING;
        return TRUE;
    }

    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, UE in idle state, drop the data\n",  __func__, m_uniqueId);
        return TRUE;
    }   

    return m_harqEntity->handleAckNack(pHIDci0Header, pHiPdu, this);
}

// --------------------------------------------
void UeTerminal::allocateDlHarqCallback(UInt16 harqProcessNum, BOOL result) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqProcessNum = %d, result = %d\n",  __func__,
        m_uniqueId, m_state, harqProcessNum, result);
    
    if (result == TRUE) {
        if (m_state == MSG3_SENT) {
            m_state = MSG4_DCI_RECVD;
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, recv Contention resolution DCI pdu, change state to %d\n",  __func__, m_uniqueId, m_state);
        } else if (m_state == MSG4_ACK_SENT){
            m_state = RRC_SETUP_DCI_RECVD;
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
void UeTerminal::dlHarqSchConfigCallback(UInt16 harqProcessNum, BOOL result) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqProcessNum = %d, result = %d\n",  __func__,
        m_uniqueId, m_state, harqProcessNum, result);

    if (result == TRUE) {
        if (m_state == MSG4_DCI_RECVD ) {
            m_state = MSG4_SCH_RECVD;
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, recv Contention resolution SCH PDU, change m_state to %d\n",  __func__, m_uniqueId, m_state);
        } else if (m_state == RRC_SETUP_DCI_RECVD) {
            m_state = RRC_SETUP_SCH_RECVD;
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, recv RRC setup SCH PDU, change m_state to %d\n",  __func__, m_uniqueId, m_state);
        } else {
//            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, recv SCH PDU (for DL DCCH data)\n",  __func__, m_uniqueId);
        }
    } else {
        // if fail to process DL SCH when RRC connection not established, terminate it
        if (m_state != RRC_CONNECTED) {
            m_state = WAIT_TERMINATING;
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
        }
    }
}

// --------------------------------------------
void UeTerminal::dlHarqReceiveCallback(UInt16 harqProcessNum, UInt8* theBuffer, UInt32 byteLen, BOOL result) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqProcessNum = %d, result = %d\n",  __func__,
        m_uniqueId, m_state, harqProcessNum, result);

    if (result == TRUE) {
        if (m_state == MSG4_SCH_RECVD) {
            if (parseContentionResolutionPdu(theBuffer, byteLen)) {
                m_state = MSG4_RECVD;
                stopContentionResolutionTimer();
                m_stsCounter->countContentionResolutionRecvd();
                // try to change rnti ??

                LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
            } else {
                m_stsCounter->countContentionResolutionInvalid();
            }
        } else if (m_state == RRC_SETUP_SCH_RECVD) {
        	UInt16 rrcMsgType = parseMacCCCHPdu(theBuffer, byteLen);
        	handleCCCHMsg(rrcMsgType);
        } else {
            parseMacPdu(theBuffer, byteLen);
        }
    } else {
        // if fail to process DL DATA when RRC connection not established, terminate it
        if (m_state != RRC_CONNECTED) {
            m_state = WAIT_TERMINATING;
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
        }
    }
}

// --------------------------------------------
void UeTerminal::handleCCCHMsg(UInt16 rrcMsgType) {
	if (rrcMsgType == 3) {
		m_state = RRC_SETUP_RECVD;
		this->stopRRCSetupTimer();
		m_stsCounter->countRRCSetupRecvd();

		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv RRC Connection Setup, change m_state to %d\n",  __func__, m_uniqueId, m_state);
	} else if (rrcMsgType == 2) {
		m_state = RRC_REJ_RECVD;
		this->stopRRCSetupTimer();
		m_stsCounter->countTestFailure();
		m_stsCounter->countRRCConnRej();
		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv RRC Connection Reject, change m_state to %d\n",  __func__, m_uniqueId, m_state);
	}  else if (rrcMsgType == 1) {
		m_state = RRC_REJ_RECVD;
		this->stopRRCSetupTimer();
		m_stsCounter->countSuccRejTest();
		m_stsCounter->countRRCRestabRej();
		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv RRC Connection Reestablishment Reject, change m_state to %d\n",  __func__, m_uniqueId, m_state);
	} else {
		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, Unsupported RRC msg, rrcMsgType = %d\n",  __func__, m_uniqueId, rrcMsgType);
		m_stsCounter->countRRCSetupInvalid();
	}
}

// --------------------------------------------
void UeTerminal::dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result) {
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
                //pTddHarqPduInd->harqBuffer[0] = 4;
                m_state = MSG4_ACK_SENT;
                this->startRRCSetupTimer();
            } else if (m_state == RRC_SETUP_RECVD) {
                //pTddHarqPduInd->harqBuffer[0] = 4;
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
void UeTerminal::allocateUlHarqCallback(UInt16 harqId, BOOL result) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqId = %d, result = %d\n",  __func__,
        m_uniqueId, m_state, harqId, result);

    if (result == TRUE) {
        if (m_state == RRC_SETUP_COMPLETE_SR_SENT) {
            m_state = RRC_SETUP_COMPLETE_SR_DCI0_RECVD;
        } else if (m_state == RRC_SETUP_COMPLETE_BSR_ACK_RECVD) {            
            m_state = RRC_SETUP_COMPLETE_DCI0_RECVD;   
        } else {
            // TODO
            if ((m_state >= RRC_SETUP_COMPLETE_SENT) && (m_state <= RRC_CONNECTED)) {
                m_subState = SUB_ST_DCI0_RECVD;
            }
        }

        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
    }
}

// -------------------------------------------------------
UInt8 gMcs2TBSIndex[29] = {  
    0, 1, 2, 3 , 4, 5, 6, 7, 8, 9,
    10, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 19, 20, 21, 22, 23, 24, 25, 26};

UInt16 gTbSizeTable[27][8] = {
    { 16,  32,	 56,   88,	 120,  152,  176, 208   },                                
    { 24,  56,	 88,   144,	 176,  208,  224, 256   },
    { 32,  72,	 144,  176,  208,  256,  296, 328   },
    { 40,  104,	 176,  208,  256,  328,  392, 440   },
    { 56,  120,	 208,  256,  328,  408,  488, 552   },
    { 72,  144,	 224,  328,  424,  504,  600, 680   },
    { 328, 176,	 256,  392,  504,  600,  712, 808   },
    { 104, 224,	 328,  472,  584,  712,  840, 968   },
    { 120, 256,	 392,  536,  680,  808,  968, 1096  },
    { 136, 296,	 456,  616,  776,  936,  1096, 1256 },
    { 144, 328,	 504,  680,  872,  1032, 1224, 1384 },
    { 176, 376,	 584,  776,  1000, 1192, 1384, 1608 },
    { 208, 440,	 680,  904,  1128, 1352, 1608, 1800 },
    { 224, 488,	 744,  1000, 1256, 1544, 1800, 2024 },
    { 256, 552,	 840,  1128, 1416, 1736, 1992, 2280 },
    { 280, 600,	 904,  1224, 1544, 1800, 2152, 2472 },
    { 328, 632,	 968,  1288, 1608, 1928, 2280, 2600 },
    { 336, 696,	 1064, 1416, 1800, 2152, 2536, 2856 },
    { 376, 776,	 1160, 1544, 1992, 2344, 2792, 3112 },
    { 408, 840,	 1288, 1736, 2152, 2600, 2984, 3496 },
    { 440, 904,	 1384, 1864, 2344, 2792, 3240, 3752 },
    { 488, 1000, 1480, 1992, 2472, 2984, 3496, 4008 },
    { 520, 1064, 1608, 2152, 2664, 3240, 3752, 4264 },
    { 552, 1128, 1736, 2280, 2856, 3496, 4008, 4584 },
    { 584, 1192, 1800, 2408, 2984, 3624, 4264, 4968 },
    { 616, 1256, 1864, 2536, 3112, 3752, 4392, 5160 },
    { 712, 1480, 2216, 2984, 3752, 4392, 5160, 5992 }
};
// --------------------------------------------
UInt16 UeTerminal::getUlTBSize(UInt8 numRb, UInt8 mcs) {
    if (mcs > 28 || numRb > 8) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, not supported yet\n", __func__, m_uniqueId);
        return 0;
    }

    UInt8 tbsIndex = gMcs2TBSIndex[mcs];

    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, numRb = %d, mcs = %d, tbSize = %d\n", __func__, m_uniqueId, numRb, mcs, gTbSizeTable[tbsIndex][numRb-1]);

    return gTbSizeTable[tbsIndex][numRb-1];
}

// --------------------------------------------
void UeTerminal::ulHarqSendCallback(UInt16 harqId, UInt8 numRb, UInt8 mcs, UInt8& ueState) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqId = %d, numRb = %d, mcs = %d\n",  __func__,
        m_uniqueId, m_state, harqId, numRb, mcs);

    BOOL result = TRUE;

    if (RRC_SETUP_COMPLETE_DCI0_RECVD == ueState) {
        buildRRCSetupComplete();
        buildCrcData(0);
        m_stsCounter->countRRCSetupComplCrcSent();
    } else if (RRC_SETUP_COMPLETE_SR_DCI0_RECVD == ueState) {
        // if (numRb < 4) {  // TODO need to consider both mcs and numRb
        //     // send BSR to request more UL resource for sending ul data
        //     buildBSRAndData();
        //     buildCrcData(0);      
        // } else {
        //     buildRRCSetupComplete();
        //     buildCrcData(0);
        //     m_stsCounter->countRRCSetupComplCrcSent();
        // }
        UInt16 tbSize = getUlTBSize(numRb, mcs);
        if ((tbSize >= (RRC_SETUP_COMPLETE_LENGTH << 3)) || (numRb > 8)) {
            buildRRCSetupComplete();
            buildCrcData(0);
            m_stsCounter->countRRCSetupComplCrcSent();
        } else {
            // send BSR to request more UL resource for sending ul data
            buildBSRAndData();
            buildCrcData(0);  
        }
    } else {
        // TODO
        if (ueState >= RRC_SETUP_COMPLETE_SENT && ueState <= RRC_CONNECTED) {
            // if (numRb == 1) {
            //     if (!m_triggerIdRsp && m_triggerRlcStatusPdu) {
            //         // only need to send RLC status pdu, 1 RB is enough
            //         buildBSRAndData(TRUE);
            //         buildCrcData(0);  
            //     } else {
            //         // need to send identity resp, may also send m_triggerRlcStatusPdu 
            //         // send short BSR to request more UL resource for sending ul data 
            //         buildBSRAndData();
            //         buildCrcData(0);  
            //     }
            // } else {
            //     // more than 1 RB allocated, send ul data now
            //     buildBSRAndData(TRUE);
            //     buildCrcData(0); 
            // }    

            UInt16 tbSize = getUlTBSize(numRb, mcs);
            if ((tbSize >= (IDENTITY_MSG_LENGTH << 3)) || (numRb > 8)) {
                // send ul data now
                buildBSRAndData(TRUE);
                buildCrcData(0); 
            } else {
                // need to send identity resp, may also send m_triggerRlcStatusPdu 
                // send short BSR to request more UL resource for sending ul data 
                buildBSRAndData();
                buildCrcData(0);  
            }
        } else if (ueState >= RRC_RELEASING) {
            buildBSRAndData(TRUE);
            buildCrcData(0);     
        }
    }

    if (result == TRUE) {
        if (m_state == RRC_SETUP_COMPLETE_SR_DCI0_RECVD) {
        	if (numRb < 4) {  // TODO need to consider both mcs and numRb
        		m_state = RRC_SETUP_COMPLETE_BSR_SENT;
        	} else {
        		m_state = RRC_SETUP_COMPLETE_SENT;
        	}

        } else if (m_state == RRC_SETUP_COMPLETE_DCI0_RECVD) {
            m_state = RRC_SETUP_COMPLETE_SENT;
        } else {
            if ((m_state >= RRC_SETUP_COMPLETE_SENT) && (m_state <= RRC_CONNECTED)) {
                if (m_subState == SUB_ST_DCI0_RECVD) {
                    m_subState = SUB_ST_TB_SENT;
                }
            }
        }

        ueState = m_state;
    }

    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
}

// --------------------------------------------
void UeTerminal::ulHarqResultCallback(UInt16 harqId, BOOL result, UInt8 ueState) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, result = %d, ueState = %d, harqId = %d, \n",  __func__,
        m_uniqueId, result, ueState, harqId);

    if (result == TRUE) {
        if (ueState == RRC_SETUP_COMPLETE_SENT && m_state < RRC_CONNECTED) {
            this->stopT300();
            m_state = RRC_CONNECTED;
        } else if (ueState == RRC_SETUP_COMPLETE_BSR_SENT) {
            m_state = RRC_SETUP_COMPLETE_BSR_ACK_RECVD;
        } else {
            if ((m_state >= RRC_SETUP_COMPLETE_SENT) && (m_state <= RRC_CONNECTED)) {
                m_subState = SUB_ST_IDLE;
            }
        }
    } else {
        if (ueState == RRC_SETUP_COMPLETE_SENT) {
            // receive nack for RRC setup complete,
            // terminate the connection later
            this->stopT300();
            m_state = WAIT_TERMINATING;
        } else {
            // TODO
        }
    }

    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
}

// --------------------------------------------
void UeTerminal::ulHarqTimeoutCallback(UInt16 harqId, UInt8 ueState) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, ueState = %d,  harqId = %d, \n",  __func__, m_uniqueId, ueState, harqId);

    // TODO retransmit UL TB??

    if (ueState == RRC_SETUP_COMPLETE_SENT) {
        // fail to receive harq ack for RRC setup complete,
        // terminate the connection later
        this->stopT300();
        this->m_stsCounter->countTestFailure();
        m_state = WAIT_TERMINATING;
    } else if (ueState == RRC_SETUP_COMPLETE_BSR_SENT) {
        this->stopT300();
        m_state = WAIT_TERMINATING;
        this->m_stsCounter->countTestFailure();
    } else {
        // TODO
        m_state = WAIT_TERMINATING;
        this->m_stsCounter->countTestFailure();
    }

    LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
}

// --------------------------------------------
void UeTerminal::handleDlConfigReq(FAPI_dlConfigRequest_st* pDlConfigReq) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle FAPI_dlConfigRequest_st\n",  __func__, m_uniqueId);
    
}
        
// --------------------------------------------
void UeTerminal::handleUlConfigReq(FAPI_ulConfigRequest_st* pUlConfigReq) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, handle FAPI_ulConfigRequest_st\n",  __func__, m_uniqueId);
}

// --------------------------------------------
BOOL UeTerminal::parseRarPdu(UInt8* data, UInt32 pduLen) {
	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, pduLen = %d: \n",  __func__, m_uniqueId, pduLen);
#ifdef OS_LINUX
    LOG_BUFFER(data, pduLen);
#endif

    if (pduLen < MIN_RAR_LENGTH) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid rar length\n",  __func__, m_uniqueId);
        return FALSE;
    }

    UInt16 i = 0;
    UInt8 ext = data[i] & 0x80;
    BOOL result = FALSE;
    RandomAccessResp* rarMsg = &m_dlSchMsg->rar;

    // refer to 36.321 6.1.5 [http://www.sharetechnote.com/html/MAC_LTE.html#MAC_PDU_Structure_RAR]
    do {
        if (data[i] & 0x40) {
            // RAPID header
            rarMsg->rapid = data[i] & 0x3f;
            ++i;
            // printf("data[%d] = %02x\n", i, data[i]);
            rarMsg->ta = ((data[i] & 0x7f) << 4) | ((data[i+1] & 0xf0) >> 4);
            ++i;
            // printf("data[%d] = %02x\n", i, data[i]);
            UInt32 ulGrantVal = ((data[i] & 0x0f) << 16) | (data[i+1] << 8) | data[i+2];
            i += 3;
            // printf("data[%d] = %02x\n", i, data[i]);
            rarMsg->tcRnti = (data[i] << 8) | data[i+1];
            ++i;
            
            // refer to 36.213, section 6.2
            rarMsg->ulGrant.hoppingFlag = (ulGrantVal >> 19) & 0x01;
            rarMsg->ulGrant.rbMap = (ulGrantVal >> 9) & 0x3ff;
            rarMsg->ulGrant.coding = (ulGrantVal >> 5) & 0x0f;
            rarMsg->ulGrant.tpc = (ulGrantVal >> 2) & 0x03;
            rarMsg->ulGrant.ulDelay = (ulGrantVal >> 1) & 0x01;
            rarMsg->ulGrant.cqiReq = ulGrantVal & 0x01;

            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, rapid = %d, ta = %d, tcRnti = %d\n", 
                __func__, m_uniqueId, rarMsg->rapid, rarMsg->ta, rarMsg->tcRnti);
            
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, tpc = %d, ulDelay = %d, cqiReq = %d\n", 
                __func__, m_uniqueId, rarMsg->ulGrant.tpc, rarMsg->ulGrant.ulDelay, rarMsg->ulGrant.cqiReq);

            if (validateRar(rarMsg)) {
                result = TRUE;
                break;
            }
        } else {
        	rarMsg->rapid = data[i] & 0x0f;
            i++;
            rarMsg->ta = ((data[i] & 0x7f) << 4) | ((data[i+1] & 0xf0) >> 4);
            ++i;
            // printf("data[%d] = %02x\n", i, data[i]);
            UInt32 ulGrantVal = ((data[i] & 0x0f) << 16) | (data[i+1] << 8) | data[i+2];
            i += 3;
            // printf("data[%d] = %02x\n", i, data[i]);
            rarMsg->tcRnti = (data[i] << 8) | data[i+1];
            ++i;

            // refer to 36.213, section 6.2
            rarMsg->ulGrant.hoppingFlag = (ulGrantVal >> 19) & 0x01;
            rarMsg->ulGrant.rbMap = (ulGrantVal >> 9) & 0x3ff;
            rarMsg->ulGrant.coding = (ulGrantVal >> 5) & 0x0f;
            rarMsg->ulGrant.tpc = (ulGrantVal >> 2) & 0x03;
            rarMsg->ulGrant.ulDelay = (ulGrantVal >> 1) & 0x01;
            rarMsg->ulGrant.cqiReq = ulGrantVal & 0x01;

            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, BI = %d, ta = %d, tcRnti = %d\n",
                __func__, m_uniqueId, rarMsg->rapid, rarMsg->ta, rarMsg->tcRnti);

            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, tpc = %d, ulDelay = %d, cqiReq = %d\n",
                __func__, m_uniqueId, rarMsg->ulGrant.tpc, rarMsg->ulGrant.ulDelay, rarMsg->ulGrant.cqiReq);

            result = TRUE;

            m_biRecvd = TRUE;
        }
    } while (ext == 1 && i < pduLen);

    // if (result) {
    //     LOG_DBG(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], ulGrant = 0x%04x, tcRnti = %d\n",  __func__, 
    //         m_ueId, m_raRnti, rarMsg->ulGrant, rarMsg->tcRnti);
    // }

    return result;
}


// --------------------------------------------
BOOL UeTerminal::parseContentionResolutionPdu(UInt8* data, UInt32 pduLen) {
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
    UInt8 ueId = ((data[i] & 0x0f) << 4) | ((data[i+1] & 0xf0) >> 4);
    ++i;
    if (ueId != m_ueId) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, invalid parsed ueId = %d\n",  __func__, m_uniqueId, ueId);
        return result;
    }
    UInt32 randomValue = ((data[i] & 0x0f) << 28) | ((data[i+1] & 0xf0) << 20) | ((data[i+1] & 0x0f) << 20) 
        | ((data[i+2] & 0xf0) << 12) | ((data[i+2] & 0x0f) << 12) | ((data[i+3] & 0xf0) << 4) 
        | ((data[i+3] & 0x0f) << 4) | ((data[i+4] & 0xf0) >> 4);

    if (randomValue != m_msg3.randomValue) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, invalid parsed randomValue = %d\n",  __func__, m_uniqueId, randomValue);
        return result;
    }

    result = TRUE;

    return result;
}

// --------------------------------------------
UInt16 UeTerminal::parseMacCCCHPdu(UInt8* data, UInt32 pduLen) {
	LOG_DBG(UE_LOGGER_NAME, "[%s], %s,CCCH pduLen = %d\n",  __func__, m_uniqueId, pduLen);

#ifdef OS_LINUX
    LOG_BUFFER(data, pduLen);
#endif
    // 3f 00 68 12 98 0f a9 a0 19 83 b0 fa 73 3e 45 e5 c9 23 f8 60 c0 10 20 01 22 00 
    // or 20 16 1f 68 12 98 09 fd d0 01 83 b0 99 98 67 96 a4 b3 21 83 99 02 00 04 60 00 
    // refer to 36.321 6.2.1, [http://blog.sina.com.cn/s/blog_5eba1ad10100gwj8.html]
    // refer to lteMacCCCH.c, line 1156
    // if the SDU length is less than 128 bytes, pading one byte ahead, else padding 2 bytes

    UInt8* pStart = data;
    UInt8* pEnd = data + pduLen;
    UInt8 lcId;
    UInt8 ext = 1;
    UInt16 ccchSduLen = 0;
    while ((pStart < pEnd) && (ext == 1)) {
        lcId = *pStart & 0x1f;
        ext = (*pStart & 0x20) >> 5;
        pStart++;

        if (lcId == lc_padding) {
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, Recv padding sdu\n",  __func__, m_uniqueId);
        } else if (lcId == lc_ccch) {
            if (ccchSduLen != 0) {
                LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, only one CCCH SDU supported\n",  __func__, m_uniqueId);
                return FALSE;
            }
            if (ext) {
                UInt8 fbit = *pStart & 0x80;
                if (fbit == 0) {
                    ccchSduLen = (*pStart++) & 0x7f;
                } else {
                    UInt8 tempX = *pStart++;
                    UInt8 tempY = *pStart++;
                    ccchSduLen = ((tempX & 0x7f) << 8) | tempY;
                }
            } else {
                // The last MAC Header does not need length indicator
                ccchSduLen = pEnd - pStart; 
            }
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, Recv CCCH sdu, length = %d\n",  __func__, m_uniqueId, ccchSduLen);
        }
    }

    // simple parse the RRC msg type, check if it is RRC setup
    UInt16 rrcMsgType = (*pStart & 0xe0) >> 5;
    return rrcMsgType;
//    if (rrcMsgType == 3) {
//        result = TRUE;
//    } else if (rrcMsgType = 2) {
//
//    } else {
//        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, invalid rrcMsgType = %d\n",  __func__, m_uniqueId, rrcMsgType);
//    }
//
//    return result;
}

// ------------------------------------------------------
void UeTerminal::setSfnSfForSR(BOOL isRetransmitSR) {
    // refer to 36.213 Table 10.1-5
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, m_srConfigIndex = %d\n",  __func__, m_uniqueId, m_srConfigIndex);

    if (!isRetransmitSR) {
        m_srCounter = 0;
    } else {
        m_srCounter++;
    }

#ifdef TDD_CONFIG
    // only valid for TDD DL/UL config 2

    // if m_srConfigIndex = 17, ul sf = 2, ul sfn = 0, 2, 4, 6, ...
    // if m_srConfigIndex = 72, ul sf = 7, ul sfn = 3, 7, 11, 15, ...

    SInt8 nOffset = 0;
    if ((m_srConfigIndex >= 15) && (m_srConfigIndex <= 34)) {
        m_srPeriodicity = 20;
        nOffset = m_srConfigIndex - 15;
        if (nOffset == 2 || nOffset == 12) {
        	m_srSf = 2;
        } else if(nOffset == 7 || nOffset == 17) {
        	m_srSf = 7;
        } else {
        	LOG_WARN(UE_LOGGER_NAME, "[%s], %s, unsupported m_srConfigIndex = %d\n",  __func__, m_uniqueId, m_srConfigIndex);
        }
    } else if ((m_srConfigIndex >= 35) && (m_srConfigIndex <= 74)) {
        m_srPeriodicity = 40;
        nOffset = m_srConfigIndex - 35;
        if (nOffset == 2 || nOffset == 12 || nOffset == 22 || nOffset == 32) {
        	m_srSf = 2;
        } else if(nOffset == 7 || nOffset == 17 || nOffset == 27 || nOffset == 37) {
        	m_srSf = 7;
        } else {
        	LOG_WARN(UE_LOGGER_NAME, "[%s], %s, unsupported m_srConfigIndex = %d\n",  __func__, m_uniqueId, m_srConfigIndex);
        }
    } else {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, unimpletemented, TODO\n",  __func__, m_uniqueId);
        return;
    }
    
#else
    SInt8 nOffset = 0;
    if ((m_srConfigIndex >= 15) && (m_srConfigIndex <= 34)) {
    	m_srPeriodicity = 20;
    	nOffset = m_srConfigIndex - 15;
    } else if ((m_srConfigIndex >= 35) && (m_srConfigIndex <= 74)) {
    	m_srPeriodicity = 40;
    	nOffset = m_srConfigIndex - 35;
    } else {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, unimpletemented, TODO\n",  __func__, m_uniqueId);
        return;
    }

    m_srSf = nOffset % 10;

#endif

    m_srSfn = m_sfn;
    if (m_sf >= m_srSf) {
        m_srSfn = (m_sfn + 1) % 1024;
    }

    SInt8 calcNs = m_srSf * 2 / 2;
    SInt8 tmp = (m_srSfn * 10 + calcNs - nOffset) % m_srPeriodicity;
    if (tmp != 0) {
        if (tmp < 0) {
            tmp = 0 - tmp;
        }
        m_srSfn = (m_srSfn + (m_srPeriodicity - tmp) / 10) % 1024;
    }

    LOG_INFO(UE_LOGGER_NAME, "[%s], %s, m_srSfnSf = %d.%d\n", __func__, m_uniqueId, m_srSfn, m_srSf);

    m_needSendSR = TRUE;
}

// --------------------------------------------
void UeTerminal::parseMacPdu(UInt8* data, UInt32 pduLen) {
    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, pduLen = %d\n",  __func__, m_uniqueId, pduLen);

#ifdef OS_LINUX
    LOG_BUFFER(data, pduLen);
#endif

    // identity request 
    // 21 02 21 0b 1f 00 04 88 00 00 0a 00 18 3a a8 08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

    UInt8* pStart = data;
    UInt8* pEnd = data + pduLen;
//    UInt32 numBytesParsed = 0;
    UInt8 lcId;
    UInt8 ext = 1;
    UInt16 sumLength = 0;
//    BOOL isPaddingBegin = ((*pStart & 0x1f) == PADDING);
    vector<LcIdItem> lcIdItemVect;

    while ((pStart < pEnd) && (ext == 1)) {
        lcId = *pStart & 0x1f;
        ext = (*pStart & 0x20) >> 5;
        pStart++;

        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, lcId = 0x%02x, ext = %d\n",  __func__, m_uniqueId, lcId, ext);

        switch (lcId) {
            case LCID1:
            case LCID2:
            case LCID3:
            case LCID4:
            case LCID5:
            case LCID6:
            case LCID7:
            case LCID8:
            case LCID9:
            case LCID10:
            {
                LcIdItem item;
                item.lcId = lcId;
                if (ext) {
                    UInt8 fbit = *pStart & 0x80;
                    if (fbit == 0) {
                        item.length = (*pStart++) & 0x7f;
                    } else {
                        UInt8 tempX = *pStart++;
                        UInt8 tempY = *pStart++;
                        item.length = ((tempX & 0x7f) << 8) | tempY;
                    }
                    sumLength += item.length;
                } else {
                    // The last MAC Header does not need length indicator
                    item.length = pEnd - pStart - sumLength; 
                }
                lcIdItemVect.push_back(item);
                break;
            }

            case TA_CMD:
            {
                LOG_DBG(UE_LOGGER_NAME, "[%s], %s, Recv TA Command, ta = %d\n",  __func__, m_uniqueId, *(pStart+1));
                m_stsCounter->countTACmdRecvd();
                break;
            }

            case PADDING:
            {
                LOG_DBG(UE_LOGGER_NAME, "[%s], %s, Recv padding sdu\n",  __func__, m_uniqueId);
                break;
            }

            default:
            {
                LOG_DBG(UE_LOGGER_NAME, "[%s], %s, unimpletemented, TODO\n",  __func__, m_uniqueId);
            }
        }
    }

    UInt16 numLcId = lcIdItemVect.size();
    for (UInt32 i=0; i<numLcId; i++) {
        LcIdItem* item = &lcIdItemVect[i];           
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, parse lcid = %d, length = %d\n",  __func__, m_uniqueId, item->lcId, item->length);

        item->buffer = pStart; 
        pStart += item->length;  
        m_rlcLayer->handleRxAMDPdu(item->buffer, item->length);       
    }    

    lcIdItemVect.clear();
}

// --------------------------------------------
void UeTerminal::requestUlResource() {
    // check if there is available ul harq process to send ul data 
    // if exists, no need to send SR. TODO, need to calc max ul length 
    // according RB number and modulation 
    if (m_harqEntity->getNumPreparedUlHarqProcess() || m_needSendSR || isSRSent()) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, no need to send SR\n",  __func__, m_uniqueId);
    } else {
        setSfnSfForSR();
        // LOG_DBG(UE_LOGGER_NAME, "[%s], [UE: %d], [RA-RNTI: %d], [RNTI: %d], need to send SR, todo\n",  __func__, 
        //     m_ueId, m_raRnti, m_rnti);
    }
}

// --------------------------------------------
void UeTerminal::rrcCallback(UInt32 msgType, RrcMsg* msg) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, msgType = %d\n",  __func__, m_uniqueId, msgType);

    switch (msgType) {
        case IDENTITY_REQUEST:
        {
            m_stsCounter->countIdentityRequestRecvd();
            m_triggerIdRsp = TRUE;
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Identity Request, will send Identity Resp later\n",  __func__, m_uniqueId);
            
            // TODO if receive identity request again ??

            requestUlResource();
            
            break;
        }

        case ATTACH_REJECT:
        {
            m_stsCounter->countAttachRejectRecvd();
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
void UeTerminal::rlcCallback(UInt8* statusPdu, UInt32 length) {
	LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, need to send RLC status PDU, length = %d\n",  __func__, m_uniqueId, length);
    
    if (length != 2) {
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, Only support 2 bytes format RLC Status PDU now\n",  __func__, m_uniqueId);
        return;
    }

    m_triggerRlcStatusPdu = TRUE;
    requestUlResource();
    memcpy(&m_rlcStatusPdu, statusPdu, length);
}
