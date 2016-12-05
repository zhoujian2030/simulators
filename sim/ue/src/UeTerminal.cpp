/*
 * UeTerminal.cpp
 *
 *  Created on: Nov 05, 2016
 *      Author: j.zhou
 */

#include <stddef.h>
#include "UeTerminal.h"
#include "CLogger.h"
#include "UeMacAPI.h"
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
UeTerminal::UeTerminal(UInt8 ueId, UInt16 raRnti, UeMacAPI* ueMacAPI) 
: m_ueMacAPI(ueMacAPI), m_ueId(ueId), m_raRnti(raRnti), m_preamble(raRnti), m_ta(1), m_state(IDLE),
  m_subState(IDLE), m_rachSf(SUBFRAME_SENT_RACH), m_rachSfn(0)
{
    m_harqEntity = new HarqEntity(NUM_UL_HARQ_PROCESS, NUM_DL_HARQ_PROCESS);

    m_t300Value = -1;
    m_contResolutionTValue = -1;
    m_srTValue = -1;
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
}

// ------------------------------------------------------
void UeTerminal::reset() {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d]\n", m_ueId, m_raRnti);
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
}

// --------------------------------------------
UeTerminal::~UeTerminal() {

}

// --------------------------------------------
void UeTerminal::schedule(UInt16 sfn, UInt8 sf, UeScheduler* pUeScheduler) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], %d.%d\n", 
        m_ueId, m_raRnti, sfn, sf);
    
    m_sfn = sfn;
    m_sf = sf;   

    if (isT300Expired()) {
        this->reset();
        StsCounter::getInstance()->countTestFailure();
        pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], %d.%d, T300 expired, reset state to IDLE\n", 
            m_ueId, m_raRnti, sfn, sf);
    }

    this->scheduleRach(pUeScheduler);
    this->scheduleMsg3(pUeScheduler);
    this->scheduleSR(pUeScheduler);
    this->scheduleDCCH(pUeScheduler);
    this->processDlHarq(pUeScheduler);
    // TODO
}

// --------------------------------------------
void UeTerminal::handleDeleteUeReq() {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], %d.%d\n", m_ueId, m_raRnti, m_rnti, m_sfn, m_sf);

    if (m_state == RRC_RELEASING) {
        StsCounter::getInstance()->countTestSuccess();
    } else {
        StsCounter::getInstance()->countTestFailure();
    }
    m_state = WAIT_TERMINATING;
}

// --------------------------------------------
void UeTerminal::scheduleRach(UeScheduler* pUeScheduler) {    
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], %d.%d\n", m_ueId, m_raRnti, m_sfn, m_sf);

    if (m_state == IDLE && m_sf == m_rachSf) {
        m_rachSfn = m_sfn;
        m_raTicks = 0;

        // reset m_rnti value
        m_rnti = 0; 

        // only send rach in specisl subframe 1 according SIB2
        UInt32 msgLen = 0;
        FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ueMacAPI->getRachBuffer();
        pL1Api->lenVendorSpecific = 0;
        pL1Api->msgId = PHY_UL_RACH_INDICATION;

        FAPI_rachIndication_st* pRachInd = (FAPI_rachIndication_st *)&pL1Api->msgBody[0];
        pRachInd->sfnsf = ( (m_rachSfn) << 4) | ( (m_rachSf) & 0xf);        
        pRachInd->numOfPreamble += 1;
        UInt32 rachHeaderLen = offsetof(FAPI_rachIndication_st, rachPduInfo);

        FAPI_rachPduIndication_st* pRachPduInd = (FAPI_rachPduIndication_st*)&pRachInd->rachPduInfo[pRachInd->numOfPreamble-1];
        pRachPduInd->rnti = m_raRnti;
        pRachPduInd->preamble = m_preamble;
        pRachPduInd->timingAdvance = m_ta;  
        
        msgLen += sizeof(FAPI_rachPduIndication_st);
        pL1Api->msgLen += msgLen;

        m_ueMacAPI->addRachDataLength(msgLen);
        if (pRachInd->numOfPreamble == 1) {
            m_ueMacAPI->addRachDataLength(FAPI_HEADER_LENGTH + rachHeaderLen);
            pL1Api->msgLen += rachHeaderLen;
        }

        this->startT300();
        m_state = MSG1_SENT;

        StsCounter::getInstance()->countRachSent();

        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], compose rach indication, msgLen = %d\n", 
            m_ueId, m_raRnti, pL1Api->msgLen);
    } else {
        // check RACH timer
        if (m_state == MSG1_SENT || m_state == MSG2_DCI_RECVD || m_state == MSG2_SCH_RECVD) {
            m_raTicks++;
            if (m_raTicks <= (subframeDelayAfterRachSent + raResponseWindowSize)) {
                return;
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], RACH timeout\n", m_ueId, m_raRnti);
                StsCounter::getInstance()->countRachTimeout();
                this->reset();
                StsCounter::getInstance()->countTestFailure();
                pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
            }
        }
    }
}

// --------------------------------------------
void UeTerminal::scheduleMsg3(UeScheduler* pUeScheduler) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], %d.%d\n", m_ueId, m_raRnti, m_rnti, m_sfn, m_sf);
    if (m_state == MSG2_RECVD) {
        // to send MSG3 in the UL subframe
        if (m_sfn == m_msg3Sfn && m_sf == m_msg3Sf) {
            buildMsg3Data();
            buildCrcData(0);
            StsCounter::getInstance()->countMsg3CrcSent();
            m_state = MSG3_SENT;
            startContentionResolutionTimer(); 
        } else {
            // TODO exception in case frame lost
        }
    } else if (m_state == MSG3_SENT) {
        if (processContentionResolutionTimer()) {
            this->reset();
            StsCounter::getInstance()->countTestFailure();
            pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
        }
    } else {
        // TODO
    }
}

// --------------------------------------------
void UeTerminal::scheduleSR(UeScheduler* pUeScheduler) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], %d.%d\n", m_ueId, m_raRnti, m_rnti, m_sfn, m_sf);

    if (m_needSendSR) {
        if (m_sfn == m_srSfn && m_sf == m_srSf) {
            UInt32 msgLen = 0;
            FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ueMacAPI->getSrBuffer();
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

            m_ueMacAPI->addSrDataLength(msgLen);
            if (pSrInd->numOfSr == 1) {
                m_ueMacAPI->addSrDataLength(FAPI_HEADER_LENGTH + srHeaderLen);
                pL1Api->msgLen += srHeaderLen;
            }

            if (m_state == RRC_SETUP_ACK_SENT) {
                m_state = RRC_SETUP_COMPLETE_SR_SENT;
            } else {
                // TODO
            }

            startSRTimer();
            m_needSendSR = FALSE;

            StsCounter::getInstance()->countSRSent();

            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], compose SR, msgLen = %d\n", 
                m_ueId, m_raRnti, m_rnti, pL1Api->msgLen);
        } else {
            // TODO exception in case frame lost
        }
    } else {
        if (processSRTimer()) {
            this->reset();
            StsCounter::getInstance()->countTestFailure();
            pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
        }
    }
}

// --------------------------------------------
void UeTerminal::scheduleDCCH(UeScheduler* pUeScheduler) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_state = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state);

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
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], terminate the connection\n", 
                m_ueId, m_raRnti, m_rnti);
            this->reset();
            pUeScheduler->resetUeTerminal(m_rnti, m_ueId);
            break;
        }

        default:
        {
            if ((m_state >= RRC_SETUP_COMPLETE_SENT) && (m_state <= RRC_CONNECTED)) {
                LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], handle ul harq in state = %d\n", 
                    m_ueId, m_raRnti, m_rnti, m_state);
                m_harqEntity->send(this);
                m_harqEntity->calcAndProcessUlHarqTimer(this);
            }

            if (m_state == RRC_RELEASING) {
                LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], handle ul harq in state = %d\n", 
                    m_ueId, m_raRnti, m_rnti, m_state);

                m_harqEntity->send(this);
                m_harqEntity->calcAndProcessUlHarqTimer(this);
            }
            
            break;
        }
    }
}

// --------------------------------------------
void UeTerminal::processDlHarq(UeScheduler* pUeScheduler) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], %d.%d\n", m_ueId, m_raRnti, m_rnti, m_sfn, m_sf);
    m_harqEntity->sendAck(m_sfn, m_sf, this);
}

// --------------------------------------------------------
BOOL UeTerminal::processContentionResolutionTimer() {
    if (m_contResolutionTValue < 0) {
        return FALSE;
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_contResolutionTValue = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_contResolutionTValue);

    if (m_contResolutionTValue == 0) {
        // contention resolution timeout
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], Contention Resolution timeout\n", m_ueId, m_raRnti, m_rnti);
        StsCounter::getInstance()->countContentionResolutionTimeout();
        return TRUE;
    } 
        
    m_contResolutionTValue--;
    return FALSE;
}

// -------------------------------------------------------
BOOL UeTerminal::processSRTimer() {
    if (m_srTValue < 0) {
        return FALSE;
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_srTValue = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_srTValue);
    
    if (m_srTValue == 0) {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], SR timeout\n", m_ueId, m_raRnti, m_rnti);
        StsCounter::getInstance()->countSRTimeout();
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

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_bsrTValue = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_bsrTValue);
    
    if (m_bsrTValue == 0) {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], BSR timeout\n", m_ueId, m_raRnti, m_rnti);
        //StsCounter::getInstance()->countSRTimeout();
        return TRUE;
    }

    m_bsrTValue--;
    return FALSE;
}

// --------------------------------------------
void UeTerminal::buildCrcData(UInt8 crcFlag) {
    UInt32 msgLen = 0;

    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ueMacAPI->getCrcBuffer();
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

    m_ueMacAPI->addCrcDataLength(msgLen);
    if (pCrcInd->numOfCrc == 1) {
        m_ueMacAPI->addCrcDataLength(FAPI_HEADER_LENGTH + crcHeaderLen);
        pL1Api->msgLen += crcHeaderLen;
    }       

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], compose crc indication, msgLen = %d\n", 
        m_ueId, m_raRnti, m_rnti, pL1Api->msgLen);
}

// --------------------------------------------
void UeTerminal::buildMsg3Data() {
    m_msg3.randomValue++;

    UInt32 msgLen = 0;
    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ueMacAPI->getSchBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UL_RX_ULSCH_INDICATION;

    FAPI_rxULSCHIndication_st *pULSchInd = (FAPI_rxULSCHIndication_st*)&pL1Api->msgBody[0];
    pULSchInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
    pULSchInd->numOfPdu += 1;
    // msgLen += offsetof(FAPI_rxULSCHIndication_st, ulDataPduInfo);

    FAPI_ulDataPduIndication_st *pUlDataPduInd;
    
    if (pULSchInd->numOfPdu == 1) {
        UInt32 schHeaderLen = offsetof(FAPI_rxULSCHIndication_st, ulDataPduInfo);
        m_ueMacAPI->addSchDataLength(FAPI_HEADER_LENGTH + schHeaderLen);
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
    m_ueMacAPI->addSchDataLength(msgLen);

    m_ueMacAPI->addSchPduData(msg3Buffer, pUlDataPduInd->length);

    StsCounter::getInstance()->countMsg3Sent();

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], compose MSG3, msgLen = %d\n", 
        m_ueId, m_raRnti, m_rnti, pL1Api->msgLen);
}

#define MAX_UL_TB_LENGTH 128
// --------------------------------------------
void UeTerminal::buildBSR(BOOL isLongBSR) {
    UInt32 msgLen = 0;
    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ueMacAPI->getSchBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UL_RX_ULSCH_INDICATION;

    FAPI_rxULSCHIndication_st *pULSchInd = (FAPI_rxULSCHIndication_st*)&pL1Api->msgBody[0];
    pULSchInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
    pULSchInd->numOfPdu += 1;

    FAPI_ulDataPduIndication_st *pUlDataPduInd;
    
    if (pULSchInd->numOfPdu == 1) {
        UInt32 schHeaderLen = offsetof(FAPI_rxULSCHIndication_st, ulDataPduInfo);
        m_ueMacAPI->addSchDataLength(FAPI_HEADER_LENGTH + schHeaderLen);
        pL1Api->msgLen += schHeaderLen;
        pUlDataPduInd = (FAPI_ulDataPduIndication_st *)&pULSchInd->ulDataPduInfo[0];
    } else {
        pUlDataPduInd = (FAPI_ulDataPduIndication_st *)&pULSchInd->ulDataPduInfo[pULSchInd->numOfPdu - 1];
    }

    pUlDataPduInd->rnti = m_rnti; 
    pUlDataPduInd->length = BSR_MSG_LENGTH;
    pUlDataPduInd->ulCqi = 80;   //TBD 0 ~ 255
    pUlDataPduInd->timingAdvance = m_ta;
    pUlDataPduInd->dataOffset = 1;  // TBD

    msgLen += sizeof(FAPI_ulDataPduIndication_st);  

    if (!isLongBSR) {
        UInt8 bsr[BSR_MSG_LENGTH] = {0x3a, 0x3d, 0x1f, 0x00, 0x12};
        m_ueMacAPI->addSchPduData(bsr, pUlDataPduInd->length);
    } else {
        if (!m_triggerIdRsp && !m_triggerRlcStatusPdu) {
            UInt8 longBsr[BSR_MSG_LENGTH] = {0x3E, 0x1F, 0x00, 0x00, 0x00};
            m_ueMacAPI->addSchPduData(longBsr, pUlDataPduInd->length);
        } else {
            if (m_triggerIdRsp) {
                LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], add Identity Response\n", 
                    m_ueId, m_raRnti, m_rnti);  
                m_triggerIdRsp = FALSE;
                UInt8 identityRsp[IDENTITY_MSG_LENGTH];
                UInt32 length;
                m_rrcLayer->buildIdentityResponse(identityRsp, length);
                m_pdcpLayer->buildSrb1Header(identityRsp, length);   
                m_rlcLayer->buildRlcAMDHeader(identityRsp, length); 

                if (m_triggerRlcStatusPdu) {
                    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], add RlC status PDU\n", 
                        m_ueId, m_raRnti, m_rnti);
                    m_triggerRlcStatusPdu = FALSE;
                    UInt8 macPdu[MAX_UL_TB_LENGTH] = {0x3d, 0x21, 0x02, 0x21, 0x80,(UInt8)length, 0x1f, 0x00, m_rlcStatusPdu[0], m_rlcStatusPdu[1]};
                    memcpy(macPdu + 10, identityRsp, length);
                    pUlDataPduInd->length = MAX_UL_TB_LENGTH;
                    m_ueMacAPI->addSchPduData(macPdu, pUlDataPduInd->length);  
                } else {
                    UInt8 macPdu[IDENTITY_MSG_LENGTH] = {0x3D, 0x21, 0x80, (UInt8)length, 0x1F, 0x00};                    
                    memcpy(macPdu + 6, identityRsp, length);
                    pUlDataPduInd->length = IDENTITY_MSG_LENGTH;
                    m_ueMacAPI->addSchPduData(macPdu, pUlDataPduInd->length);  
                }
            } else {
                LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], add RlC status PDU\n", 
                    m_ueId, m_raRnti, m_rnti);
                m_triggerRlcStatusPdu = FALSE;
                UInt8 macPdu[MAX_UL_TB_LENGTH] = {0x3d, 0x21, 0x02, 0x1f, 0x00, m_rlcStatusPdu[0], m_rlcStatusPdu[1]};
                pUlDataPduInd->length = MAX_UL_TB_LENGTH;
                m_ueMacAPI->addSchPduData(macPdu, pUlDataPduInd->length);  
            }
        }
    }

    msgLen += pUlDataPduInd->length;
    pL1Api->msgLen += msgLen;
    m_ueMacAPI->addSchDataLength(msgLen);

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], compose BSR, msgLen = %d\n", 
        m_ueId, m_raRnti, m_rnti, pL1Api->msgLen);  
}

// --------------------------------------------
void UeTerminal::buildRRCSetupComplete() {    
    UInt32 msgLen = 0;
    FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ueMacAPI->getSchBuffer();
    pL1Api->lenVendorSpecific = 0;
    pL1Api->msgId = PHY_UL_RX_ULSCH_INDICATION;

    FAPI_rxULSCHIndication_st *pULSchInd = (FAPI_rxULSCHIndication_st*)&pL1Api->msgBody[0];
    pULSchInd->sfnsf = ( (m_sfn) << 4) | ( (m_sf) & 0xf);
    pULSchInd->numOfPdu += 1;
    // msgLen += offsetof(FAPI_rxULSCHIndication_st, ulDataPduInfo);

    FAPI_ulDataPduIndication_st *pUlDataPduInd;
    
    if (pULSchInd->numOfPdu == 1) {
        UInt32 schHeaderLen = offsetof(FAPI_rxULSCHIndication_st, ulDataPduInfo);
        m_ueMacAPI->addSchDataLength(FAPI_HEADER_LENGTH + schHeaderLen);
        pL1Api->msgLen += schHeaderLen;
        pUlDataPduInd = (FAPI_ulDataPduIndication_st *)&pULSchInd->ulDataPduInfo[0];
    } else {
        pUlDataPduInd = (FAPI_ulDataPduIndication_st *)&pULSchInd->ulDataPduInfo[pULSchInd->numOfPdu - 1];
    }

    pUlDataPduInd->rnti = m_rnti; 
    pUlDataPduInd->length = RRC_SETUP_COMPLETE_LENGTH;
    pUlDataPduInd->ulCqi = 80;   //TBD 0 ~ 255
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
        0x3A, 0x3D, 0x21, 0x80, 0x88, 0x1F, 0x0A, 0x00, 0xA0, 0x00, 0x00,
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

    msgLen += pUlDataPduInd->length;
    pL1Api->msgLen += msgLen;
    m_ueMacAPI->addSchDataLength(msgLen);

    m_ueMacAPI->addSchPduData(rrcSetupCompl, pUlDataPduInd->length);

    StsCounter::getInstance()->countRRCSetupComplSent();

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], compose RRC setup complete, msgLen = %d\n", 
        m_ueId, m_raRnti, m_rnti, pL1Api->msgLen);
}

// --------------------------------------------
void UeTerminal::handleDlDciPdu(FAPI_dlConfigRequest_st* pDlConfigHeader, FAPI_dciDLPduInfo_st* pDlDciPdu) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], handle FAPI_dciDLPduInfo_st, m_state = %d\n", 
        m_ueId, m_raRnti, m_state);
    
    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], UE in idle state, drop the data\n", m_ueId, m_raRnti);
        return;
    }
    
    UInt8 sf  = pDlConfigHeader->sfnsf & 0x000f;
    UInt16 sfn  = (pDlConfigHeader->sfnsf & 0xfff0) >> 4;

    UInt16 rnti = pDlDciPdu->rnti;
    if (rnti == m_raRnti) {
        // recv RAR DCI PDU
        if (validateDlSfnSf(sfn, sf)) {
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], recv RAR DCI PDU\n", m_ueId, m_raRnti);
            m_state = MSG2_DCI_RECVD;
        } else {
            LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], invalid time config for RAR DCI pdu\n", 
                m_ueId, m_raRnti);
        }
    } else {
        // recv other DL DCI PDU
        if (validateDlSfnSf(sfn, sf)) {
            m_harqEntity->allocateDlHarqProcess(pDlConfigHeader->sfnsf, pDlDciPdu, this);
            // if (m_state == MSG3_SENT) {
            //     m_state = MSG4_DCI_RECVD;
            //     LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv Contention resolution DCI pdu\n",
            //         m_ueId, m_raRnti, rnti);
            // } else if (m_state == MSG4_ACK_SENT){
            //     m_state = RRC_SETUP_DCI_RECVD;
            //     LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv RRC setup DCI pdu\n",
            //         m_ueId, m_raRnti, rnti);
            // } else {
            //     // TODO
            //     LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], other DL DCI pdu\n",
            //         m_ueId, m_raRnti, rnti);
            // }
        } else {
            LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], invalid time config for DL DCI pdu\n", 
                m_ueId, m_raRnti, rnti);
        }
    }
}

// --------------------------------------------
void UeTerminal::handleDlSchPdu(FAPI_dlConfigRequest_st* pDlConfigHeader, FAPI_dlSCHConfigPDUInfo_st* pDlSchPdu) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], handle FAPI_dlSCHConfigPDUInfo_st, m_state = %d\n", 
        m_ueId, m_raRnti, m_state);  

    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], UE in idle state, drop the data\n", m_ueId, m_raRnti);
        return;
    }  

    UInt16 rnti = pDlSchPdu->rnti;
    if (rnti == m_raRnti) {
        // recv RAR SCH PDU
        m_provSf  = pDlConfigHeader->sfnsf & 0x000f;
        m_provSfn  = (pDlConfigHeader->sfnsf & 0xfff0) >> 4;
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], provSfnSf = %d.%d, curSfnSf = %d.%d\n", m_ueId, m_raRnti, m_provSfn, m_provSf, m_sfn, m_sf);
        if (validateDlSfnSf(m_provSfn, m_provSf)) {
            if (m_state == MSG2_DCI_RECVD ) {
                m_state = MSG2_SCH_RECVD;
                LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], recv RAR SCH PDU\n", m_ueId, m_raRnti);
            } else {
                LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], invalid RAR SCH pdu in m_state = %d\n", m_ueId, m_raRnti, m_state);
            }
        } else {
            LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], invalid time config for RAR SCH pdu\n", 
                m_ueId, m_raRnti);
        }
    } else {
        // recv other SCH PDU
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [C-RNTI: %d], mcs = %d\n", m_ueId, m_rnti, pDlSchPdu->mcs);
        m_harqEntity->handleDlSchConfig(pDlConfigHeader->sfnsf, pDlSchPdu, this);
        // if (validateDlSfnSf(m_provSfn, m_provSf)) {
        //     LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], rbCoding = %d, transmissionScheme = %d\n", 
        //         m_ueId, m_raRnti, rnti, pDlSchPdu->rbCoding, pDlSchPdu->transmissionScheme);
        //     if (m_state == MSG4_DCI_RECVD ) {
        //         m_state = MSG4_SCH_RECVD;
        //         LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv Contention resolution SCH PDU\n", m_ueId, m_raRnti, rnti);
        //     } else if (m_state == RRC_SETUP_DCI_RECVD) {
        //         m_state = RRC_SETUP_SCH_RECVD;
        //         LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv RRC setup SCH PDU\n", m_ueId, m_raRnti, rnti);
        //     } else {
        //         // TODO
        //         LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv other SCH PDU, not implemented\n", m_ueId, m_raRnti, rnti);
        //     }
        // } else {
        //     LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], invalid time config for other SCH pdu\n", 
        //         m_ueId, m_raRnti);
        // }
    } 
}

// --------------------------------------------
void UeTerminal::handleDlTxData(FAPI_dlDataTxRequest_st* pDlDataTxHeader, FAPI_dlTLVInfo_st *pDlTlv, UeScheduler* pUeScheduler) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], handle FAPI_dlTLVInfo_st, length = %d, m_state = %d\n", 
        m_ueId, m_raRnti, pDlTlv->tagLen, m_state); 
    
    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], UE in idle state, drop the data\n", m_ueId, m_raRnti);
        return;
    } 

    UInt32 byteLen = pDlTlv->tagLen;
    UInt8* data = (UInt8*)pDlTlv->value;

    if (m_state == MSG2_SCH_RECVD) {
        UInt16 sfnsf = (m_provSfn << 4) | m_provSf;
        if (sfnsf != pDlDataTxHeader->sfnsf) {
            LOG_WARN(UE_LOGGER_NAME, "Invalid sfnsf = 0x%04x\n", pDlDataTxHeader->sfnsf);
            return;
        }

        if (parseRarPdu(data, byteLen)) {
            m_state = MSG2_RECVD;
            this->setSfnSfForMsg3();
            this->m_rnti = m_dlSchMsg->rar.tcRnti;
            pUeScheduler->updateRntiUeIdMap(m_rnti, m_ueId);
            StsCounter::getInstance()->countRarRecvd();
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], recv RAR msg and change m_state to %d, MSG3 will be sent in %d.%d\n", 
                m_ueId, m_raRnti, m_state, m_msg3Sfn, m_msg3Sf); 
        } else {
            StsCounter::getInstance()->countRarInvalid();
            LOG_ERROR(UE_LOGGER_NAME, "Fail to parse RAR\n");
        }   
    } else {
        m_harqEntity->receive(pDlDataTxHeader->sfnsf, data, byteLen, this);
    }
}

// --------------------------------------------
void UeTerminal::handleUlSchPdu(FAPI_ulConfigRequest_st* pUlConfigHeader, FAPI_ulSCHPduInfo_st* pUlSchPdu) {
    UInt16 sfn = (pUlConfigHeader->sfnsf & 0xfff0) >> 4;
    UInt8 sf = pUlConfigHeader->sfnsf & 0x0f;

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], configure to receive UL SCH data in tick %d.%d\n", 
        m_ueId, m_raRnti, m_rnti, sfn, sf); 

    LOG_DEBUG(UE_LOGGER_NAME, "numOfRB = %d, cyclicShift2forDMRS = %d\n", pUlSchPdu->numOfRB, pUlSchPdu->cyclicShift2forDMRS);
    
    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], UE in idle state, drop the data\n", m_ueId, m_raRnti);
        return;
    } 

    if (m_state == MSG2_RECVD) {
        if (sfn != m_msg3Sfn || sf != m_msg3Sf) {
            LOG_ERROR(UE_LOGGER_NAME, "Invalid frame configured for MSG3\n");
            return;
        }        
        // only count this msg for statistics, UE will send MSG3 no matter MAC 
        // send this UL config or not
        StsCounter::getInstance()->countMsg3ULCfgRecvd();
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], UL config to receive MSG3\n", 
            m_ueId, m_raRnti, m_rnti); 
    } else {
        m_harqEntity->handleUlSchConfig(pUlConfigHeader->sfnsf, (void*)pUlSchPdu, this);
    }
}

// --------------------------------------------
void UeTerminal::handleDci0Pdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlDCIPduInfo_st* pDci0Pdu) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], handle DCI0, m_state = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state); 

    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], UE in idle state, drop the data\n", m_ueId, m_raRnti);
        return;
    }

    // UInt8 sf  = pHIDci0Header->sfnsf & 0x000f;
    // UInt16 sfn  = (pHIDci0Header->sfnsf & 0xfff0) >> 4;
    UInt8 dciFormat = pDci0Pdu->ulDCIFormat;
    // UInt16 harqId = (pDci0Pdu->rbStart << 8) | pDci0Pdu->cyclicShift2_forDMRS;

    if (dciFormat == FAPI_UL_DCI_FORMAT_0) {
        UInt8 numOfRB = pDci0Pdu->numOfRB;
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], receive FAPI_UL_DCI_FORMAT_0, rbStart = %d, "
            "cyclicShift2_forDMRS = %d, numOfRB = %d\n", 
            m_ueId, m_raRnti, m_rnti, pDci0Pdu->rbStart, pDci0Pdu->cyclicShift2_forDMRS, numOfRB); 

        // eNb allocates the UL resource, stop the timer and prepare to send RRC setup complete            
        if (m_state == RRC_SETUP_COMPLETE_SR_SENT) {                 
            m_harqEntity->allocateUlHarqProcess(pHIDci0Header, pDci0Pdu, this);
            // stop the SR timer even it fails to allocate harq process for sending UL data
            stopSRTimer();                
            // StsCounter::getInstance()->countRRCSetupComplDCI0Recvd();
            
        } else if (m_state == RRC_SETUP_COMPLETE_BSR_ACK_RECVD) {  
            m_harqEntity->allocateUlHarqProcess(pHIDci0Header, pDci0Pdu, this);       
            stopBSRTimer();
            StsCounter::getInstance()->countRRCSetupComplDCI0Recvd();
        } else {
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], TBD state = %d\n", 
                m_ueId, m_raRnti, m_rnti, m_state); 
            // TODO
            m_harqEntity->allocateUlHarqProcess(pHIDci0Header, pDci0Pdu, this); 
        }
    } else {
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], handle DCI0, unsupported dciFormat = %d\n", 
            m_ueId, m_raRnti, m_rnti, dciFormat); 
    }
}

// --------------------------------------------
void UeTerminal::handleHIPdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlHiPduInfo_st* pHiPdu) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], handle HI, m_state = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state); 

    if (m_state == IDLE) {
        LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], UE in idle state, drop the data\n", m_ueId, m_raRnti);
        return;
    }   

    m_harqEntity->handleAckNack(pHIDci0Header, pHiPdu, this);
}

// --------------------------------------------
void UeTerminal::allocateDlHarqCallback(UInt16 harqProcessNum, BOOL result) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_state = %d, harqProcessNum = %d, result = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state, harqProcessNum, result);
    
    if (result == TRUE) {
        if (m_state == MSG3_SENT) {
            m_state = MSG4_DCI_RECVD;
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv Contention resolution DCI pdu\n",
                m_ueId, m_raRnti, m_rnti);
        } else if (m_state == MSG4_ACK_SENT){
            m_state = RRC_SETUP_DCI_RECVD;
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv RRC setup DCI pdu\n",
                m_ueId, m_raRnti, m_rnti);
        } else {
            // TODO
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], other DL DCI pdu\n",
                m_ueId, m_raRnti, m_rnti);
        }
    } else {
        // if fail to process DL DCI when RRC connection not established, terminate it
        if (m_state != RRC_CONNECTED) {
            m_state = WAIT_TERMINATING;
        }
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], change m_state to %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state);
}

// --------------------------------------------
void UeTerminal::dlHarqSchConfigCallback(UInt16 harqProcessNum, BOOL result) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_state = %d, harqProcessNum = %d, result = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state, harqProcessNum, result);

    if (result == TRUE) {
        if (m_state == MSG4_DCI_RECVD ) {
            m_state = MSG4_SCH_RECVD;
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv Contention resolution SCH PDU\n", m_ueId, m_raRnti, m_rnti);
        } else if (m_state == RRC_SETUP_DCI_RECVD) {
            m_state = RRC_SETUP_SCH_RECVD;
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv RRC setup SCH PDU\n", m_ueId, m_raRnti, m_rnti);
        } else {
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [C-RNTI: %d], recv SCH PDU (for DL DCCH data)\n", m_ueId, m_raRnti, m_rnti);
        }
    } else {
        // if fail to process DL SCH when RRC connection not established, terminate it
        if (m_state != RRC_CONNECTED) {
            m_state = WAIT_TERMINATING;
        }
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], change m_state to %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state);
}

// --------------------------------------------
void UeTerminal::dlHarqReceiveCallback(UInt16 harqProcessNum, UInt8* theBuffer, UInt32 byteLen, BOOL result) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_state = %d, harqProcessNum = %d, result = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state, harqProcessNum, result);

    if (result == TRUE) {
        if (m_state == MSG4_SCH_RECVD) {
            if (parseContentionResolutionPdu(theBuffer, byteLen)) {
                m_state = MSG4_RECVD;
                stopContentionResolutionTimer();
                StsCounter::getInstance()->countContentionResolutionRecvd();
                // try to change rnti ??
            } else {
                StsCounter::getInstance()->countContentionResolutionInvalid();
            }
        } else if (m_state == RRC_SETUP_SCH_RECVD) {
            if (parseRRCSetupPdu(theBuffer, byteLen)) {
                m_state = RRC_SETUP_RECVD;
                StsCounter::getInstance()->countRRCSetupRecvd();
            } else {
                StsCounter::getInstance()->countRRCSetupInvalid();
            }    
        } else {
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], recv DCCH data (need parsed later)\n", 
                m_ueId, m_raRnti, m_rnti);

            parseMacPdu(theBuffer, byteLen);
        }
    } else {
        // if fail to process DL DATA when RRC connection not established, terminate it
        if (m_state != RRC_CONNECTED) {
            m_state = WAIT_TERMINATING;
        }
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], change m_state to %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state);
}

// --------------------------------------------
void UeTerminal::dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_state = %d, harqProcessNum = %d, result = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state, harqProcessNum, result);
    
    if (result == TRUE) {
        UInt32 msgLen = 0;
        FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ueMacAPI->getHarqBuffer();
        FAPI_harqIndication_st* pHarqInd = (FAPI_harqIndication_st *)&pL1Api->msgBody[0];

        // for both bundling mode and special bundling mode, UE send ack for all DL TB or nack for all DL TB
        if (firstAck) {
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

            m_ueMacAPI->addHarqDataLength(msgLen);
            if (pHarqInd->numOfHarq == 1) {
                m_ueMacAPI->addHarqDataLength(FAPI_HEADER_LENGTH + harqHeaderLen);
                pL1Api->msgLen += harqHeaderLen;
            }       
            
            if (m_state == MSG4_RECVD) {
                m_state = MSG4_ACK_SENT;
            } else if (m_state == RRC_SETUP_RECVD) {
                m_state = RRC_SETUP_ACK_SENT;
                this->setSfnSfForSR();
            } else {
                // TODO                
            }
        } else {
            FAPI_tddHarqPduIndication_st* pTddHarqPduInd = (FAPI_tddHarqPduIndication_st*)&pHarqInd->harqPduInfo[pHarqInd->numOfHarq - 1]; 
            pTddHarqPduInd->numOfAckNack += 1;   
        }

        // it counts number of ACK needs to sent for DL TB, 
        // not counts the actual harq messages (FAPI_harqIndication_st) sents
        StsCounter::getInstance()->countHarqAckSent();
    }
}

// --------------------------------------------
void UeTerminal::allocateUlHarqCallback(UInt16 harqId, BOOL result) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_state = %d, harqId = %d, result = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state, harqId, result);

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

        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], change m_state to %d\n", 
            m_ueId, m_raRnti, m_rnti, m_state);
    }
}

// --------------------------------------------
void UeTerminal::ulHarqSendCallback(UInt16 harqId, BOOL result) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], m_state = %d, harqId = %d, result = %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state, harqId, result);

    if (result == TRUE) {
        if (m_state == RRC_SETUP_COMPLETE_SR_DCI0_RECVD) {
            m_state = RRC_SETUP_COMPLETE_BSR_SENT;
        } else if (m_state == RRC_SETUP_COMPLETE_DCI0_RECVD) {
            m_state = RRC_SETUP_COMPLETE_SENT;
        } else {
            if ((m_state >= RRC_SETUP_COMPLETE_SENT) && (m_state <= RRC_CONNECTED)) {
                if (m_subState == SUB_ST_DCI0_RECVD) {
                    m_subState = SUB_ST_TB_SENT;
                }
            }
        }
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], change m_state to %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state);
}

// --------------------------------------------
void UeTerminal::ulHarqResultCallback(UInt16 harqId, BOOL result, UInt8 ueState) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], result = %d, ueState = %d, harqId = %d, \n", 
        m_ueId, m_raRnti, m_rnti, result, ueState, harqId);

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

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], change m_state to %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state);
}

// --------------------------------------------
void UeTerminal::ulHarqTimeoutCallback(UInt16 harqId, UInt8 ueState) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], ueState = %d,  harqId = %d, \n", 
        m_ueId, m_raRnti, m_rnti, ueState, harqId);

    if (ueState == RRC_SETUP_COMPLETE_SENT) {
        // fail to receive harq ack for RRC setup complete,
        // terminate the connection later
        this->stopT300();
        m_state = WAIT_TERMINATING;
    } else if (ueState == RRC_SETUP_COMPLETE_BSR_SENT) {
        this->stopT300();
        m_state = WAIT_TERMINATING;
    } else {
        // TODO
        m_state = WAIT_TERMINATING;
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], change m_state to %d\n", 
        m_ueId, m_raRnti, m_rnti, m_state);
}

// --------------------------------------------
void UeTerminal::handleDlConfigReq(FAPI_dlConfigRequest_st* pDlConfigReq) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], handle FAPI_dlConfigRequest_st\n", 
        m_ueId, m_raRnti);
    
}
        
// --------------------------------------------
void UeTerminal::handleUlConfigReq(FAPI_ulConfigRequest_st* pUlConfigReq) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], handle FAPI_ulConfigRequest_st\n", 
        m_ueId, m_raRnti);
}

// --------------------------------------------
BOOL UeTerminal::parseRarPdu(UInt8* data, UInt32 pduLen) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], pduLen = %d: \n", 
        m_ueId, m_raRnti, pduLen);
    for (UInt32 i=0; i<pduLen; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");

    if (pduLen < MIN_RAR_LENGTH) {
        LOG_ERROR(UE_LOGGER_NAME, "Invalid rar length\n");
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
            rarMsg->ta = ((data[i] & 0x7f) << 4) | ((data[++i] & 0xf0) >> 4);
            // printf("data[%d] = %02x\n", i, data[i]);
            rarMsg->ulGrant = ((data[i] & 0x0f) << 16) | (data[++i] << 8) | data[++i];
            ++i;
            // printf("data[%d] = %02x\n", i, data[i]);
            rarMsg->tcRnti = (data[i] << 8) | data[++i];

            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], rapid = %d, ta = %d, ulGrant = 0x%04x, tcRnti = %d\n", 
                m_ueId, m_raRnti, rarMsg->rapid, rarMsg->ta, rarMsg->ulGrant, rarMsg->tcRnti);

            if (validateRar(rarMsg)) {
                result = TRUE;
                break;
            }
        } else {
            // BI header
            // TODO
            i++;
        }
    } while (ext == 1 && i < pduLen);

    // if (result) {
    //     LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], ulGrant = 0x%04x, tcRnti = %d\n", 
    //         m_ueId, m_raRnti, rarMsg->ulGrant, rarMsg->tcRnti);
    // }

    return result;
}


// --------------------------------------------
BOOL UeTerminal::parseContentionResolutionPdu(UInt8* data, UInt32 pduLen) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], pduLen = %d: \n", 
        m_ueId, m_raRnti, m_rnti, pduLen);

    for (UInt32 i=0; i<pduLen; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");

    BOOL result = FALSE;
    UInt8 i = 0;

    // refer to 36.321 6.1.3.4 []http://www.sharetechnote.com/html/Handbook_LTE_MAC_CE.html]
    if (pduLen != CONTENTION_RESOLUTION_LENGTH) {
        LOG_ERROR(UE_LOGGER_NAME, "Invalid contention resolution length\n");
        return result;
    }
    UInt8 lcId = data[i];
    if (lcId != lc_contention_resolution) {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], invalid lcId = %d\n",
             m_ueId, m_raRnti, m_rnti, lcId);
        return result;
    }
    // parse and validate
    ++i;
    UInt8 ueId = ((data[i] & 0x0f) << 4) | ((data[++i] & 0xf0) >> 4);
    if (ueId != m_ueId) {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], invalid parsed ueId = %d\n",
             m_ueId, m_raRnti, m_rnti, ueId);
        return result;
    }
    UInt32 randomValue = ((data[i] & 0x0f) << 28) | ((data[++i] & 0xf0) << 20) | ((data[i] & 0x0f) << 20) 
        | ((data[++i] & 0xf0) << 12) | ((data[i] & 0x0f) << 12) | ((data[++i] & 0xf0) << 4) 
        | ((data[i] & 0x0f) << 4) | ((data[++i] & 0xf0) >> 4);

    if (randomValue != m_msg3.randomValue) {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], invalid parsed randomValue = %d\n",
             m_ueId, m_raRnti, m_rnti, randomValue);
        return result;
    }

    result = TRUE;

    return result;
}

// --------------------------------------------
BOOL UeTerminal::parseRRCSetupPdu(UInt8* data, UInt32 pduLen) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], pduLen = %d: \n", 
        m_ueId, m_raRnti, m_rnti, pduLen);

    for (UInt32 i=0; i<pduLen; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
    // 3f 00 68 12 98 0f a9 a0 19 83 b0 fa 73 3e 45 e5 c9 23 f8 60 c0 10 20 01 22 00 
    // refer to 36.321 6.2.1, [http://blog.sina.com.cn/s/blog_5eba1ad10100gwj8.html]
    // refer to lteMacCCCH.c, line 1156
    // if the SDU length is less than 128 bytes, pading one byte ahead, else padding 2 bytes

    BOOL result = FALSE;
    UInt32 i = 0;
    if (pduLen < (128 + 2)) {
        // one byte padding format
        if (((data[i] & 0x1f) != lc_padding) || ((data[i] & 0x20) == 0) || (data[++i] != lc_ccch)) {
            LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], Invalid RRC setup msg\n",
                m_ueId, m_raRnti, m_rnti);
            return result;
        }
    } else {
        // two bytes padding format
        if (((data[i] & 0x1f) != lc_padding) || ((data[i] & 0x20) == 0) 
            || ((data[++i] & 0x1f) != lc_padding) || ((data[i] & 0x20) == 0) || (data[++i] != lc_ccch)) {
            LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], Invalid RRC setup msg\n",
                m_ueId, m_raRnti, m_rnti);
            return result;
        }
    }

    // simple parse the RRC msg type, check if it is RRC setup
    UInt32 rrcMsgType = (data[++i] & 0xe0) >> 5;
    if (rrcMsgType == 3) {
        result = TRUE;
    } else {
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], invalid rrcMsgType = %d\n", 
            m_ueId, m_raRnti, m_rnti, rrcMsgType);
    } 

    return result;    
}

// --------------------------------------------
void UeTerminal::parseMacPdu(UInt8* data, UInt32 pduLen) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], pduLen = %d\n", 
        m_ueId, m_raRnti, m_rnti, pduLen);

    for (UInt32 i=0; i<pduLen; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");

    // identity request 
    // 21 02 21 0b 1f 00 04 88 00 00 0a 00 18 3a a8 08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

    UInt8* pStart = data;
    UInt8* pEnd = data + pduLen;
    UInt32 numBytesParsed = 0;
    UInt8 lcId;
    UInt8 ext = 1;
    UInt16 length = 0;
    vector<LcIdItem> lcIdItemVect;

    while ((pStart < pEnd) && (ext == 1)) {
        lcId = *pStart & 0x1f;
        ext = (*pStart & 0x20) >> 5;
        pStart++;

        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], lcId = 0x%02x, ext = %d\n", 
            m_ueId, m_raRnti, m_rnti, lcId, ext);

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
                UInt8 fbit = *pStart & 0x80;
                if (fbit == 0) {
                    item.length = (*pStart++) & 0x7f;
                } else {
                    item.length = (((*pStart++) & 0x7f) << 8) | (*pStart++);
                }
                lcIdItemVect.push_back(item);
                break;
            }

            case TA_CMD:
            {
                LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], Recv TA Command\n", 
                    m_ueId, m_raRnti, m_rnti);
                break;
            }

            case PADDING:
            {
                LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], Recv padding sdu\n", 
                    m_ueId, m_raRnti, m_rnti);
                break;
            }

            default:
            {
                LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], unimpletemented, TODO\n", 
                    m_ueId, m_raRnti, m_rnti);
            }
        }
    }

    UInt16 numLcId = lcIdItemVect.size();
    for (UInt32 i=0; i<numLcId; i++) {
        LcIdItem* item = &lcIdItemVect[i];           
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], parse lcid = %d, length = %d\n", 
            m_ueId, m_raRnti, m_rnti, item->lcId, item->length); 

        item->buffer = pStart; 
        pStart += item->length;  
        m_rlcLayer->handleRxAMDPdu(item->buffer, item->length);       
    }    

    lcIdItemVect.clear();
}


// --------------------------------------------
void UeTerminal::rrcCallback(UInt32 msgType, RrcMsg* msg) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], msgType = %d\n", 
        m_ueId, m_raRnti, m_rnti, msgType);

    switch (msgType) {
        case IDENTITY_REQUEST:
        {
            StsCounter::getInstance()->countIdentityRequestRecvd();
            m_triggerIdRsp = TRUE;
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], recv Identity Request, will send Identity Resp later\n", 
                m_ueId, m_raRnti, m_rnti);
            break;
        }

        case ATTACH_REJECT:
        {
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], recv Attach Reject\n", 
                m_ueId, m_raRnti, m_rnti);
            break;
        }

        case RRC_RELEASE:
        {
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], recv RRC Release, will free resource later\n", 
                m_ueId, m_raRnti, m_rnti);
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], change state from %d to %d\n", 
                m_ueId, m_raRnti, m_rnti, m_state, RRC_RELEASING);
            m_state = RRC_RELEASING;
            break;
        }

        default:
        {
            LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], Unsupported msgType\n", m_ueId, m_raRnti, m_rnti);
        }
    }
}

// --------------------------------------------
void UeTerminal::rlcCallback(UInt8* statusPdu, UInt32 length) {
    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], need to send RLC status PDU, length = %d\n", 
        m_ueId, m_raRnti, m_rnti, length);
    
    if (length != 2) {
        LOG_WARN(UE_LOGGER_NAME, "Only support 2 bytes format RLC Status PDU now\n");
        return;
    }

    m_triggerRlcStatusPdu = TRUE;
    memcpy(&m_rlcStatusPdu, statusPdu, length);
}
