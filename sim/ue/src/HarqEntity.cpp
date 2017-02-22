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
    LOG_DBG(UE_LOGGER_NAME, "[%s], reset harq entity\n", __func__);

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
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid ulSf = %d\n", __func__, pUeTerminal->m_uniqueId, ulSf);
        pUeTerminal->allocateUlHarqCallback(harqId, FALSE);
        return FALSE;
    }

    UlHarqProcess* pUlHarqProcess = 0;
    for (UInt32 i=0; i<m_numUlHarqProcess; i++) {
        if (m_ulHarqProcessList[i]->isFree()) {
            pair<map<UInt16, UInt16>::iterator, bool> result = 
                m_harqIdUlIndexMap.insert(map<UInt16, UInt16>::value_type(harqId, i));
            if (!result.second) {
                LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, harqId = %d is in used, not able to allocate\n", __func__, 
                    pUeTerminal->m_uniqueId, harqId);
                pUeTerminal->allocateUlHarqCallback(harqId, FALSE);
                return FALSE;
            }
            
            pUlHarqProcess = m_ulHarqProcessList[i];
            break;
        }
    }

    if (pUlHarqProcess != 0) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, allocate harq process success, harqId = 0x%04x\n", __func__, 
            pUeTerminal->m_uniqueId, harqId);
        pUeTerminal->allocateUlHarqCallback(harqId, TRUE);
        //
        pUlHarqProcess->prepareSending(pDci0Pdu, ulSfn, ulSf, pUeTerminal->m_state);
        return TRUE;
    }

    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, allocate harq process failed\n", __func__, pUeTerminal->m_uniqueId);
    pUeTerminal->allocateUlHarqCallback(harqId, FALSE);
    return FALSE;
}

// ------------------------------------------------
void HarqEntity::send(UeTerminal* pUeTerminal) {
    map<UInt16, UInt16>::iterator it;
    for (it = m_harqIdUlIndexMap.begin(); it != m_harqIdUlIndexMap.end(); ++it) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, execute harq process %d, harqId = 0x%04x\n", __func__, 
            pUeTerminal->m_uniqueId, it->second, it->first);
        m_ulHarqProcessList[it->second]->send(it->first, pUeTerminal);
    }
}

// ------------------------------------------------
UInt8 HarqEntity::getNumPreparedUlHarqProcess() {
    UInt8 num = 0;
    map<UInt16, UInt16>::iterator it;
    for (it = m_harqIdUlIndexMap.begin(); it != m_harqIdUlIndexMap.end(); ++it) {
        if (m_ulHarqProcessList[it->second]->isPrepareSending()) {
            num++;
        }
    }

    return num;
}

// ------------------------------------------------
BOOL HarqEntity::handleAckNack(
    FAPI_dlHiDCIPduInfo_st* pHIDci0Header, 
    FAPI_dlHiPduInfo_st* pHiPdu, 
    UeTerminal* pUeTerminal)
{
    UInt16 harqId = (pHiPdu->rbStart << 8) | pHiPdu->cyclicShift2_forDMRS;

    map<UInt16, UInt16>::iterator it = m_harqIdUlIndexMap.find(harqId);
    if (it != m_harqIdUlIndexMap.end()) {
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, handle harq ack/nack in harq process %d\n", __func__, pUeTerminal->m_uniqueId, it->second);
        
        if (m_ulHarqProcessList[it->second]->handleAckNack(harqId, pHIDci0Header->sfnsf, pHiPdu->hiValue, pUeTerminal)) {  
            // delete the record as the harq process becomes free after ACK/NACK received
            m_harqIdUlIndexMap.erase(it);
            return TRUE;
        }
    } else {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, fail to find harq process by harqId = %d\n", __func__, pUeTerminal->m_uniqueId, harqId);
    }     

    return FALSE;
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
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, handle UL SCH config in harq process %d\n", __func__, pUeTerminal->m_uniqueId, it->second);
        m_ulHarqProcessList[it->second]->handleUlSchConfig(harqId, sfnsf, pUeTerminal);  
    } else {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, fail to find harq process by harqId = 0x%04x\n", __func__, pUeTerminal->m_uniqueId, harqId);
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

        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, harqProcessNum = %d, rbCoding = %d\n", __func__, 
            pUeTerminal->m_uniqueId, pDciMsg->harqProcessNum, pDciMsg->rbCoding);

        if (index >= m_numDlHarqProcess) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, invalid harqProcessNum = %d\n", __func__, pUeTerminal->m_uniqueId, index);
            pUeTerminal->allocateDlHarqCallback(index, FALSE);
            return;
        }

        if (!m_dlHarqProcessList[index]->isFree()) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, harq process is busy, harqProcessNum = %d\n", __func__, pUeTerminal->m_uniqueId, index);
            pUeTerminal->allocateDlHarqCallback(index, FALSE);
            return;
        }

        pair<map<UInt32, UInt16>::iterator, bool> result = 
            m_harqIdDlIndexMap.insert(map<UInt32, UInt16>::value_type(harqId, index));
        if (!result.second) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, harqId = %d is in used, not able to allocate\n", __func__, pUeTerminal->m_uniqueId, harqId);
            pUeTerminal->allocateDlHarqCallback(index, FALSE);
            return;
        }

        UInt8 sf  = sfnsf & 0x000f;
        UInt16 sfn  = (sfnsf & 0xfff0) >> 4;
        m_dlHarqProcessList[index]->prepareReceiving(sfn, sf, pUeTerminal->m_state);

        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, allocate dl harq process success\n", __func__, pUeTerminal->m_uniqueId);
        pUeTerminal->allocateDlHarqCallback(index, TRUE);
        return;
    } 

    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, allocate harq process failed due to unsupported dciFormat = %d\n", __func__, 
        pUeTerminal->m_uniqueId, pDlDciPdu->dciFormat);
    pUeTerminal->allocateDlHarqCallback(0, FALSE);
}

// ------------------------------------------------
void HarqEntity::handleDlSchConfig(UInt16 sfnsf, FAPI_dlSCHConfigPDUInfo_st* pDlSchPdu, UeTerminal* pUeTerminal) {
    UInt32 harqId = sfnsf;//pDlSchPdu->rbCoding;
    map<UInt32, UInt16>::iterator it = m_harqIdDlIndexMap.find(harqId);
    if (it != m_harqIdDlIndexMap.end()) {
        m_dlHarqProcessList[it->second]->handleDlSchConfig(sfnsf, pUeTerminal);
    } else {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, fail to find harq process by harqId = 0x%02x\n", __func__, pUeTerminal->m_uniqueId, harqId);
    }
}

// -------------------------------------------    
void HarqEntity::receive(UInt16 sfnsf, UInt8* theBuffer, UInt32 byteLen, UeTerminal* pUeTerminal) {
    UInt32 harqId = sfnsf;
    map<UInt32, UInt16>::iterator it = m_harqIdDlIndexMap.find(harqId);
    if (it != m_harqIdDlIndexMap.end()) {
        m_dlHarqProcessList[it->second]->receive(sfnsf, theBuffer, byteLen, pUeTerminal);
    } else {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, fail to find harq process by harqId = 0x%02x\n", __func__, pUeTerminal->m_uniqueId, harqId);
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
: m_index(index), m_state(IDLE), m_ueState(0), m_timerValue(-1),
  m_numRb(0), m_mcs(0)
{
    
}

// ------------------------------------------------
HarqEntity::UlHarqProcess::~UlHarqProcess() {

}

// ------------------------------------------------
void HarqEntity::UlHarqProcess::free() {
    LOG_DBG(UE_LOGGER_NAME, "[%s], m_index = %d\n", __func__, m_index);
    m_state = IDLE;
    m_ueState = 0;
    m_timerValue = -1;
}

// -------------------------------------------    
void HarqEntity::UlHarqProcess::prepareSending(FAPI_dlDCIPduInfo_st* pDci0Pdu, UInt16 sfn, UInt8 sf, UInt8 ueState) {
    LOG_DBG(UE_LOGGER_NAME, "[%s], m_index = %d, sfn = %d, sf = %d, ueState = %d\n", __func__, 
            m_index, sfn, sf, ueState);
    m_state = PREPARE_SENDING;
    m_sendSfn = sfn;
    m_sendSf = sf;
    m_ueState = ueState;
    m_numRb = pDci0Pdu->numOfRB;
    m_mcs = pDci0Pdu->mcs;
}

// ------------------------------------------------
void HarqEntity::UlHarqProcess::send(UInt16 harqId, UeTerminal* pUeTerminal) {
    if (m_state != PREPARE_SENDING) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, HARQ Process is not ready to send, harqId = 0x%04x\n", __func__, pUeTerminal->m_uniqueId, harqId);
        return;
    }

    UInt16 curSfn = pUeTerminal->m_sfn;
    UInt8 curSf = pUeTerminal->m_sf;

    if (curSfn == m_sendSfn && curSf == m_sendSf) {
        pUeTerminal->ulHarqSendCallback(harqId, m_numRb, m_mcs, m_ueState);
        m_state = TB_SENT;
        startTimer(); 
    }
}

// ------------------------------------------------
BOOL HarqEntity::UlHarqProcess::handleAckNack(UInt16 harqId, UInt16 sfnsf, UInt8 hiValue, UeTerminal* pUeTerminal) {
    if (m_state != TB_SENT) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, invalid m_state = %d, harqId = %d\n", __func__, 
            pUeTerminal->m_uniqueId, m_state, harqId);
        return FALSE;
    }   

    UInt8 sf  = sfnsf & 0x000f;
    UInt16 sfn  = (sfnsf & 0xfff0) >> 4;

    LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, harqId = %d, hiValue = %d\n", __func__, pUeTerminal->m_uniqueId, harqId, hiValue);
    
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
        LOG_WARN(UE_LOGGER_NAME, "[%s], %s, recv HI in unexpected sfnsf: 0x%04x\n", __func__, pUeTerminal->m_uniqueId, sfnsf); 
        return FALSE;
    }

}

// ------------------------------------------------
void HarqEntity::UlHarqProcess::handleUlSchConfig(UInt16 harqId, UInt16 sfnsf, UeTerminal* pUeTerminal) {
    UInt16 sfn = (sfnsf & 0xfff0) >> 4;
    UInt8 sf = sfnsf & 0x0f;

    if (sfn != m_sendSfn || sf != m_sendSf) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid frame configured for UL SCH\n", __func__, pUeTerminal->m_uniqueId);
        return;
    } 

    if (UeTerminal::RRC_SETUP_COMPLETE_DCI0_RECVD == m_ueState) {
        // only count this msg for statistics, UE will send RRC setup complete no matter MAC 
        // send this UL config or not
        StsCounter::getInstance()->countRRCSetupComplULCfgRecvd();
        LOG_DBG(UE_LOGGER_NAME, "[%s], %s, UL config to receive RRC setup complete\n", __func__, pUeTerminal->m_uniqueId); 
    } else {
        // TODO
    }
}

// ------------------------------------------------
BOOL HarqEntity::UlHarqProcess::calcAndProcessTimer(UInt16 harqId, UeTerminal* pUeTerminal) {
    if (m_state != TB_SENT || m_timerValue < 0) { 
        return FALSE;
    }

    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, harqId = 0x%04x, m_timerValue = %d\n", __func__, pUeTerminal->m_uniqueId, harqId, m_timerValue);
 
    BOOL isTimeout = FALSE;
    if (m_timerValue == 0) {
        // contention resolution timeout        
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, harq timeout\n", __func__, pUeTerminal->m_uniqueId);
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
    LOG_DBG(UE_LOGGER_NAME, "[%s], m_index = %d\n", __func__, m_index);
    m_state = IDLE;
    m_ueState = 0;
}

// -------------------------------------------    
void HarqEntity::DlHarqProcess::prepareReceiving(UInt16 sfn, UInt8 sf, UInt8 ueState) {
    LOG_DBG(UE_LOGGER_NAME, "[%s], m_index = %d, sfn = %d, sf = %d, ueState = %d\n", __func__, 
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
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid frame configured for DL SCH\n", __func__, pUeTerminal->m_uniqueId);
        // DO NOT invoke callback to terminate connection, just let it timeout if no other SCH received (T300)
        return;
    } 

    pUeTerminal->dlHarqSchConfigCallback(m_index, TRUE);
}

// -------------------------------------------    
void HarqEntity::DlHarqProcess::receive(UInt16 sfnsf, UInt8* theBuffer, UInt32 byteLen, UeTerminal* pUeTerminal) {
    if (m_state != PREPARE_RECEIVING) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid m_state = %d\n", __func__, pUeTerminal->m_uniqueId, m_state);
        return;
    }

    UInt16 sfn = (sfnsf & 0xfff0) >> 4;
    UInt8 sf = sfnsf & 0x0f;

    if (sfn != m_recvSfn || sf != m_recvSf) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid frame configured for DL DATA\n", __func__, pUeTerminal->m_uniqueId);
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

    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, harq ack will be sent in %d.%d\n", __func__, pUeTerminal->m_uniqueId, m_harqAckSfn, m_harqAckSf); 
}

// -------------------------------------------    
BOOL HarqEntity::DlHarqProcess::sendAck(UInt16 sfn, UInt8 sf, BOOL isFirstAckSent, UeTerminal* pUeTerminal) {
    if (m_state != TB_RECEIVED) {
        LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Invalid m_state = %d\n", __func__, pUeTerminal->m_uniqueId, m_state);
        return FALSE;
    }

    if (sfn == m_harqAckSfn && sf == m_harqAckSf) {
        pUeTerminal->dlHarqResultCallback(m_index, 1, isFirstAckSent, TRUE);
        m_state = IDLE;
        return TRUE;
    }

    return FALSE;
}