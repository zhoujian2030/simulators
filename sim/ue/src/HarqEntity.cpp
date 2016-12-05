/*
 * HarqEntity.cpp
 *
 *  Created on: Nov 14, 2016
 *      Author: j.zhou
 */

#include "HarqEntity.h"
#include "UeTerminal.h"
#include "CLogger.h"
#include "StsCounter.h"

using namespace ue;
using namespace std;

// ------------------------------------------------
HarqEntity::HarqEntity(UInt16 numOfUlHarqProcess, UInt16 numOfDlHarqProcess) 
: m_numUlHarqProcess(numOfUlHarqProcess), m_numDlHarqProcess(numOfDlHarqProcess)
{
    m_ulHarqProcessList = new UlHarqProcess*[m_numUlHarqProcess];
    for (UInt32 i=0; i<m_numUlHarqProcess; i++) {
        m_ulHarqProcessList[i] = new UlHarqProcess(i);
    }

    m_dlHarqProcessList = new DlHarqProcess*[m_numDlHarqProcess];
    for (UInt32 i=0; i<m_numDlHarqProcess; i++) {
        m_dlHarqProcessList[i] = new DlHarqProcess(i);
    }
}

// ------------------------------------------------
HarqEntity::~HarqEntity() {
    for (UInt32 i=0; i<m_numUlHarqProcess; i++) {
        delete m_ulHarqProcessList[i];
    }
    delete []m_ulHarqProcessList;

    for (UInt32 i=0; i<m_numDlHarqProcess; i++) {
        delete m_dlHarqProcessList[i];
    }
    delete []m_dlHarqProcessList;
}

// ------------------------------------------------
void HarqEntity::reset() {
    LOG_DEBUG(UE_LOGGER_NAME, "reset harq entity\n");

    map<UInt16, UInt16>::iterator ulIter = m_harqIdUlIndexMap.begin();
    while (ulIter != m_harqIdUlIndexMap.end()) {
        m_ulHarqProcessList[ulIter->second]->free();
        ulIter++;
    }
    m_harqIdUlIndexMap.clear();

    map<UInt32, UInt16>::iterator dlIter = m_harqIdDlIndexMap.begin();
    while (dlIter != m_harqIdDlIndexMap.end()) {
        m_dlHarqProcessList[dlIter->second]->free();
        dlIter++;
    }
    m_harqIdDlIndexMap.clear();
}

// ------------------------------------------------
BOOL HarqEntity::allocateUlHarqProcess(
    FAPI_dlHiDCIPduInfo_st* pHIDci0Header, 
    FAPI_dlDCIPduInfo_st* pDci0Pdu, 
    UeTerminal* pUeTerminal) 
{
    // TODO, it will be a problem using current param as harqId
    // there is harq process id in ul sch config, but no in DCI 0
    UInt16 harqId = (pDci0Pdu->rbStart << 8) | pDci0Pdu->cyclicShift2_forDMRS;  

    // validate the provision sfnsf, only valid for TDD DL/UL config 2
    // refer to 36.213 Table 8-2 k for TDD configurations 0-6
    // refer to [http://www.sharetechnote.com/html/LTE_TDD_Overview.html#DCI0_PUSCH_Timing]
    UInt8 sf  = pHIDci0Header->sfnsf & 0x000f;
    UInt16 sfn  = (pHIDci0Header->sfnsf & 0xfff0) >> 4;
    UInt16 ulSfn = sfn;
    UInt8 ulSf = sf + 4;
    if (ulSf / 10) {
        ulSf = ulSf % 10;
        ulSfn = (ulSfn + 1) % 1024;
    }
    if (UeTerminal::m_ulSubframeList[ulSf] != 1) {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], Invalid ulSf = %d\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, ulSf);
        pUeTerminal->allocateUlHarqCallback(harqId, FALSE);
        return FALSE;
    }

    UlHarqProcess* pUlHarqProcess = 0;
    for (UInt32 i=0; i<m_numUlHarqProcess; i++) {
        if (m_ulHarqProcessList[i]->isFree()) {
            pair<map<UInt16, UInt16>::iterator, bool> result = 
                m_harqIdUlIndexMap.insert(map<UInt16, UInt16>::value_type(harqId, i));
            if (!result.second) {
                LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], harqId = %d is in used, not able to allocate\n", 
                    pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId);
                pUeTerminal->allocateUlHarqCallback(harqId, FALSE);
                return FALSE;
            }
            
            pUlHarqProcess = m_ulHarqProcessList[i];
            break;
        }
    }

    if (pUlHarqProcess != 0) {
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], allocate harq process success, harqId = 0x%04x\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId);
        pUeTerminal->allocateUlHarqCallback(harqId, TRUE);
        //
        pUlHarqProcess->prepareSending(ulSfn, ulSf, pUeTerminal->m_state);
        return TRUE;
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], allocate harq process failed\n", 
        pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti);
    pUeTerminal->allocateUlHarqCallback(harqId, FALSE);
    return FALSE;
}

// ------------------------------------------------
void HarqEntity::send(UeTerminal* pUeTerminal) {
    map<UInt16, UInt16>::iterator it;
    for (it = m_harqIdUlIndexMap.begin(); it != m_harqIdUlIndexMap.end(); ++it) {
        LOG_DEBUG(UE_LOGGER_NAME, "execute harq process %d, harqId = 0x%04x\n", it->second, it->first);
        m_ulHarqProcessList[it->second]->send(it->first, pUeTerminal);
    }
}

// ------------------------------------------------
void HarqEntity::handleAckNack(
    FAPI_dlHiDCIPduInfo_st* pHIDci0Header, 
    FAPI_dlHiPduInfo_st* pHiPdu, 
    UeTerminal* pUeTerminal)
{
    UInt16 harqId = (pHiPdu->rbStart << 8) | pHiPdu->cyclicShift2_forDMRS;

    map<UInt16, UInt16>::iterator it = m_harqIdUlIndexMap.find(harqId);
    if (it != m_harqIdUlIndexMap.end()) {
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], handle harq ack/nack in harq process %d\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, it->second);
        m_ulHarqProcessList[it->second]->handleAckNack(harqId, pHIDci0Header->sfnsf, pHiPdu->hiValue, pUeTerminal);  
        // delete the record as the harq process becomes free after ACK/NACK received
        m_harqIdUlIndexMap.erase(it);
    } else {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], fail to find harq process by harqId = %d\n",
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId);
    }     
}

// -------------------------------------------    
void HarqEntity::calcAndProcessUlHarqTimer(UeTerminal* pUeTerminal) {
    map<UInt16, UInt16>::iterator it = m_harqIdUlIndexMap.begin();
    while (it != m_harqIdUlIndexMap.end()) {
        if (m_ulHarqProcessList[it->second]->calcAndProcessTimer(it->first, pUeTerminal)) {
            m_harqIdUlIndexMap.erase(it++);
        } else {
            it++;
        }
    }
}

// -------------------------------------------    
void HarqEntity::handleUlSchConfig(UInt16 sfnsf, void* ulSchPdu, UeTerminal* pUeTerminal) {
    UInt16 harqId = 0;
#ifdef FAPI_API
    FAPI_ulSCHPduInfo_st* pUlSchPdu = (FAPI_ulSCHPduInfo_st*)ulSchPdu;   
    harqId = (pUlSchPdu->rbStart << 8) | pUlSchPdu->cyclicShift2forDMRS;
#else
    // TODO
#endif

    map<UInt16, UInt16>::iterator it = m_harqIdUlIndexMap.find(harqId);
    if (it != m_harqIdUlIndexMap.end()) {
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], handle UL SCH config in harq process %d\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, it->second);
        m_ulHarqProcessList[it->second]->handleUlSchConfig(harqId, sfnsf, pUeTerminal);  
    } else {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], fail to find harq process by harqId = %d\n",
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId);
    }    
}

// ------------------------------------------------
void HarqEntity::allocateDlHarqProcess(UInt16 sfnsf, FAPI_dciDLPduInfo_st* pDlDciPdu, UeTerminal* pUeTerminal) {
    if (FAPI_DL_DCI_FORMAT_1A == pDlDciPdu->dciFormat 
        || FAPI_DL_DCI_FORMAT_1 == pDlDciPdu->dciFormat
        || FAPI_DL_DCI_FORMAT_1B == pDlDciPdu->dciFormat
        || FAPI_DL_DCI_FORMAT_1D == pDlDciPdu->dciFormat
        || FAPI_DL_DCI_FORMAT_2 == pDlDciPdu->dciFormat
        || FAPI_DL_DCI_FORMAT_2A == pDlDciPdu->dciFormat) 
    {
        FAPI_dciFormat1A_st *pDciMsg = (FAPI_dciFormat1A_st *)&(pDlDciPdu->dciPdu[0]);
        UInt32 harqId = sfnsf;//pDciMsg->rbCoding;  // TBD
        UInt16 index = pDciMsg->harqProcessNum;

        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], harqProcessNum = %d, rbCoding = %d\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, pDciMsg->harqProcessNum, pDciMsg->rbCoding);

        if (index >= m_numDlHarqProcess) {
            LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], invalid harqProcessNum = %d\n",
                pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, index);
            pUeTerminal->allocateDlHarqCallback(index, FALSE);
            return;
        }

        if (!m_dlHarqProcessList[index]->isFree()) {
            LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], harq process is busy, harqProcessNum = %d\n",
                pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, index);
            pUeTerminal->allocateDlHarqCallback(index, FALSE);
            return;
        }

        pair<map<UInt32, UInt16>::iterator, bool> result = 
            m_harqIdDlIndexMap.insert(map<UInt32, UInt16>::value_type(harqId, index));
        if (!result.second) {
            LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], harqId = %d is in used, not able to allocate\n", 
                pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId);
            pUeTerminal->allocateDlHarqCallback(index, FALSE);
            return;
        }

        UInt8 sf  = sfnsf & 0x000f;
        UInt16 sfn  = (sfnsf & 0xfff0) >> 4;
        m_dlHarqProcessList[index]->prepareReceiving(sfn, sf, pUeTerminal->m_state);

        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], allocate dl harq process success\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti);
        pUeTerminal->allocateDlHarqCallback(index, TRUE);
        return;
    } 

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], allocate harq process failed due to unsupported dciFormat = %d\n", 
        pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, pDlDciPdu->dciFormat);
    pUeTerminal->allocateDlHarqCallback(0, FALSE);
}

// ------------------------------------------------
void HarqEntity::handleDlSchConfig(UInt16 sfnsf, FAPI_dlSCHConfigPDUInfo_st* pDlSchPdu, UeTerminal* pUeTerminal) {
    UInt32 harqId = sfnsf;//pDlSchPdu->rbCoding;
    map<UInt32, UInt16>::iterator it = m_harqIdDlIndexMap.find(harqId);
    if (it != m_harqIdDlIndexMap.end()) {
        m_dlHarqProcessList[it->second]->handleDlSchConfig(sfnsf, pUeTerminal);
    } else {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], fail to find harq process by harqId = 0x%02x\n",
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId);
    }
}

// -------------------------------------------    
void HarqEntity::receive(UInt16 sfnsf, UInt8* theBuffer, UInt32 byteLen, UeTerminal* pUeTerminal) {
    UInt32 harqId = sfnsf;
    map<UInt32, UInt16>::iterator it = m_harqIdDlIndexMap.find(harqId);
    if (it != m_harqIdDlIndexMap.end()) {
        m_dlHarqProcessList[it->second]->receive(sfnsf, theBuffer, byteLen, pUeTerminal);
    } else {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], fail to find harq process by harqId = 0x%02x\n",
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId);
    }
}

// -------------------------------------------    
void HarqEntity::sendAck(UInt16 sfn, UInt8 sf, UeTerminal* pUeTerminal) {
    map<UInt32, UInt16>::iterator it = m_harqIdDlIndexMap.begin();
    BOOL isFirstAckSent = TRUE;
    while (it != m_harqIdDlIndexMap.end()) {
        if (m_dlHarqProcessList[it->second]->sendAck(sfn, sf, isFirstAckSent, pUeTerminal)) {
            if (isFirstAckSent) {
                isFirstAckSent = FALSE;
            }
            m_harqIdDlIndexMap.erase(it++);
        } else {
            it++;
        }
    }
}

// ------------------------------------------------
HarqEntity::UlHarqProcess::UlHarqProcess(UInt16 index) 
: m_index(index), m_state(IDLE), m_ueState(0), m_timerValue(-1)
{
    
}

// ------------------------------------------------
HarqEntity::UlHarqProcess::~UlHarqProcess() {

}

// ------------------------------------------------
void HarqEntity::UlHarqProcess::free() {
    LOG_DEBUG(UE_LOGGER_NAME, "m_index = %d\n", m_index);
    m_state = IDLE;
    m_ueState = 0;
    m_timerValue = -1;
}

// -------------------------------------------    
void HarqEntity::UlHarqProcess::prepareSending(UInt16 sfn, UInt8 sf, UInt8 ueState) {
    LOG_DEBUG(UE_LOGGER_NAME, "m_index = %d, sfn = %d, sf = %d, ueState = %d\n", 
            m_index, sfn, sf, ueState);
    m_state = PREPARE_SENDING;
    m_sendSfn = sfn;
    m_sendSf = sf;
    m_ueState = ueState;
}

// ------------------------------------------------
void HarqEntity::UlHarqProcess::send(UInt16 harqId, UeTerminal* pUeTerminal) {
    if (m_state != PREPARE_SENDING) {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], HARQ Process is not ready to send, harqId = 0x%04x\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId);
        return;
    }

    UInt16 curSfn = pUeTerminal->m_sfn;
    UInt8 curSf = pUeTerminal->m_sf;

    if (curSfn == m_sendSfn && curSf == m_sendSf) {
        // TODO move to do the businesss logic in UeTerminal
        if (UeTerminal::RRC_SETUP_COMPLETE_DCI0_RECVD == m_ueState) {
            pUeTerminal->buildRRCSetupComplete();
            pUeTerminal->buildCrcData(0);
            StsCounter::getInstance()->countRRCSetupComplCrcSent();    
            pUeTerminal->ulHarqSendCallback(harqId, TRUE);
            m_state = TB_SENT;
            m_ueState = pUeTerminal->m_state;
            startTimer();        
        } else if (UeTerminal::RRC_SETUP_COMPLETE_SR_DCI0_RECVD == m_ueState) {
            pUeTerminal->buildBSR();
            pUeTerminal->buildCrcData(0);
            pUeTerminal->ulHarqSendCallback(harqId, TRUE);
            m_state = TB_SENT;
            m_ueState = pUeTerminal->m_state;
            startTimer();        
        } else {
            // TODO
            if (m_ueState >= UeTerminal::RRC_SETUP_COMPLETE_SENT && m_ueState <= UeTerminal::RRC_CONNECTED) {
                pUeTerminal->buildBSR(TRUE);
                pUeTerminal->buildCrcData(0);
                pUeTerminal->ulHarqSendCallback(harqId, TRUE);
                m_state = TB_SENT;
                m_ueState = pUeTerminal->m_state;
                startTimer();        
            }

            if (m_ueState >= UeTerminal::RRC_RELEASING) {
                pUeTerminal->buildBSR(TRUE);
                pUeTerminal->buildCrcData(0);
                pUeTerminal->ulHarqSendCallback(harqId, TRUE);
                m_state = TB_SENT;
                m_ueState = pUeTerminal->m_state;
                startTimer();        
            }
        }
    }
}

// ------------------------------------------------
BOOL HarqEntity::UlHarqProcess::handleAckNack(UInt16 harqId, UInt16 sfnsf, UInt8 hiValue, UeTerminal* pUeTerminal) {
    if (m_state != TB_SENT) {
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], invalid m_state = %d, harqId = %d\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, m_state, harqId);
        return FALSE;
    }   

    UInt8 sf  = sfnsf & 0x000f;
    UInt16 sfn  = (sfnsf & 0xfff0) >> 4;

    LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], harqId = %d, hiValue = %d\n", 
        pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId, hiValue);
    
    UInt8 expectedHiSf = (m_sendSf + K_PHICH_FOR_TDD_UL_DL_CONFIG_2) % 10;
    UInt16 expectedHiSfn = (m_sendSfn + (m_sendSf + K_PHICH_FOR_TDD_UL_DL_CONFIG_2) / 10) % 1024;
    if (expectedHiSf == sf && expectedHiSfn == sfn) {
        this->stopTimer(); 
        if (hiValue == 1) {
            StsCounter::getInstance()->countHarqAckRecvd();
            pUeTerminal->ulHarqResultCallback(harqId, TRUE, m_ueState);
        } else {
            StsCounter::getInstance()->countHarqNAckRecvd();        
            pUeTerminal->ulHarqResultCallback(harqId, FALSE, m_ueState);
        }
        // reset to IDLE state without retransmitting TB even NACK
        m_state = IDLE;
        return TRUE;
    } else {
        LOG_WARN(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], recv HI in unexpected sfnsf: 0x%04x\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, sfnsf); 
        return FALSE;
    }

}

// ------------------------------------------------
void HarqEntity::UlHarqProcess::handleUlSchConfig(UInt16 harqId, UInt16 sfnsf, UeTerminal* pUeTerminal) {
    UInt16 sfn = (sfnsf & 0xfff0) >> 4;
    UInt8 sf = sfnsf & 0x0f;

    if (sfn != m_sendSfn || sf != m_sendSf) {
        LOG_ERROR(UE_LOGGER_NAME, "Invalid frame configured for UL SCH\n");
        return;
    } 

    if (UeTerminal::RRC_SETUP_COMPLETE_DCI0_RECVD == m_ueState) {
        // only count this msg for statistics, UE will send RRC setup complete no matter MAC 
        // send this UL config or not
        StsCounter::getInstance()->countRRCSetupComplULCfgRecvd();
        LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], UL config to receive RRC setup complete\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti); 
    } else {
        // TODO
    }
}

// ------------------------------------------------
BOOL HarqEntity::UlHarqProcess::calcAndProcessTimer(UInt16 harqId, UeTerminal* pUeTerminal) {
    if (m_state != TB_SENT || m_timerValue < 0) { 
        return FALSE;
    }

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], harqId = 0x%04x, m_timerValue = %d\n", 
        pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, harqId, m_timerValue);
 
    BOOL isTimeout = FALSE;
    if (m_timerValue == 0) {
        // contention resolution timeout        
        LOG_ERROR(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], harq timeout\n", 
            pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti);
        pUeTerminal->ulHarqTimeoutCallback(harqId, m_ueState);
        m_state = IDLE;
        m_ueState = 0;
        StsCounter::getInstance()->countHarqTimeout();
        isTimeout = TRUE;
    } 
        
    m_timerValue--;  
    return isTimeout;
}

// ------------------------------------------------
HarqEntity::DlHarqProcess::DlHarqProcess(UInt16 index) 
: m_index(index), m_state(IDLE), m_ueState(0)
{
    
}

// ------------------------------------------------
HarqEntity::DlHarqProcess::~DlHarqProcess() {

}

// ------------------------------------------------
void HarqEntity::DlHarqProcess::free() {    
    LOG_DEBUG(UE_LOGGER_NAME, "m_index = %d\n", m_index);
    m_state = IDLE;
    m_ueState = 0;
}

// -------------------------------------------    
void HarqEntity::DlHarqProcess::prepareReceiving(UInt16 sfn, UInt8 sf, UInt8 ueState) {
    LOG_DEBUG(UE_LOGGER_NAME, "m_index = %d, sfn = %d, sf = %d, ueState = %d\n", 
            m_index, sfn, sf, ueState);
    m_state = PREPARE_RECEIVING;
    m_recvSfn = sfn;
    m_recvSf = sf;
    m_ueState = ueState;
}

// -------------------------------------------    
void HarqEntity::DlHarqProcess::handleDlSchConfig(UInt16 sfnsf, UeTerminal* pUeTerminal) {
    UInt16 sfn = (sfnsf & 0xfff0) >> 4;
    UInt8 sf = sfnsf & 0x0f;

    if (sfn != m_recvSfn || sf != m_recvSf) {
        LOG_ERROR(UE_LOGGER_NAME, "Invalid frame configured for DL SCH\n");
        // DO NOT invoke callback to terminate connection, just let it timeout if no other SCH received (T300)
        return;
    } 

    pUeTerminal->dlHarqSchConfigCallback(m_index, TRUE);
}

// -------------------------------------------    
void HarqEntity::DlHarqProcess::receive(UInt16 sfnsf, UInt8* theBuffer, UInt32 byteLen, UeTerminal* pUeTerminal) {
    if (m_state != PREPARE_RECEIVING) {
        LOG_ERROR(UE_LOGGER_NAME, "Invalid m_state = %d\n", m_state);
        return;
    }

    UInt16 sfn = (sfnsf & 0xfff0) >> 4;
    UInt8 sf = sfnsf & 0x0f;

    if (sfn != m_recvSfn || sf != m_recvSf) {
        LOG_ERROR(UE_LOGGER_NAME, "Invalid frame configured for DL DATA\n");
        // DO NOT invoke callback to terminate connection, just let it timeout if no other DATA received (T300)
        return;
    }  

    pUeTerminal->dlHarqReceiveCallback(m_index, theBuffer, byteLen, TRUE);

    // only valid for TDD DL/UL config 2
    if (m_recvSf == 4 || m_recvSf == 5 || m_recvSf == 6 || m_recvSf == 8) {
        m_harqAckSf = 2;
        m_harqAckSfn = (m_recvSfn + 1) % 1024;
    } else {
        m_harqAckSf = 7;
        if (m_recvSf == 0 || m_recvSf == 1 || m_recvSf == 3) {                
            m_harqAckSfn = m_recvSfn;
        } else {
            m_harqAckSfn = (m_recvSfn + 1) % 1024;
        }
    }

    m_state = TB_RECEIVED;

    LOG_DEBUG(UE_LOGGER_NAME, "[UE: %d], [RA-RNTI: %d], [RNTI: %d], harq ack will be sent in %d.%d\n", 
        pUeTerminal->m_ueId, pUeTerminal->m_raRnti, pUeTerminal->m_rnti, m_harqAckSfn, m_harqAckSf); 
}

// -------------------------------------------    
BOOL HarqEntity::DlHarqProcess::sendAck(UInt16 sfn, UInt8 sf, BOOL isFirstAckSent, UeTerminal* pUeTerminal) {
    if (m_state != TB_RECEIVED) {
        LOG_ERROR(UE_LOGGER_NAME, "Invalid m_state = %d\n", m_state);
        return FALSE;
    }

    if (sfn == m_harqAckSfn && sf == m_harqAckSf) {
        pUeTerminal->dlHarqResultCallback(m_index, 1, isFirstAckSent, TRUE);
        m_state = IDLE;
        return TRUE;
    }

    return FALSE;
}