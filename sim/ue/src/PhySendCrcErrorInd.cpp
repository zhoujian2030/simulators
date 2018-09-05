/*
 * PhySendCrcErrorInd.cpp
 *
 *  Created on: Sep 4, 2018
 *      Author: J.ZH
 */

#include "PhySendCrcErrorInd.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif
#include "PhyMacAPI.h"
#include "UeScheduler.h"
#include "StsCounter.h"
#include "HarqEntity.h"

using namespace ue;

// --------------------------------------------
PhySendCrcErrorInd::PhySendCrcErrorInd(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, preamble, phyMacAPI, stsCounter)
{
	m_numHarqTx = 0;
}

// --------------------------------------------
PhySendCrcErrorInd::~PhySendCrcErrorInd() {

}

// --------------------------------------------
void PhySendCrcErrorInd::handleDci0Pdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlDCIPduInfo_st* pDci0Pdu) {
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
        m_numHarqTx++;
        LOG_INFO(UE_LOGGER_NAME, "[%s], %s, receive FAPI_UL_DCI_FORMAT_0, rbStart = %d, "
            "cyclicShift2_forDMRS = %d, numOfRB = %d, m_numHarqTx = %d\n",  __func__, m_uniqueId, pDci0Pdu->rbStart,
            pDci0Pdu->cyclicShift2_forDMRS, numOfRB, m_numHarqTx);

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
        	if (m_state == CRC_ERR_IND_SENT) {
        		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, UL HARQ retransmit, state = %d\n",  __func__, m_uniqueId, m_state);
        		UInt16 harqId = (pDci0Pdu->rbStart << 8) | pDci0Pdu->cyclicShift2_forDMRS;
        		m_harqEntity->freeUlHarqProcess(harqId);
        		m_state = RRC_SETUP_COMPLETE_SR_DCI0_RECVD;
        	} else if (isSRSent()) {
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
void PhySendCrcErrorInd::ulHarqSendCallback(UInt16 harqId, UInt8 numRb, UInt8 mcs, UInt8& ueState) {
	LOG_INFO(UE_LOGGER_NAME, "[%s], %s, m_state = %d, harqId = %d, numRb = %d, mcs = %d, send crc err indication\n",  __func__,
        m_uniqueId, m_state, harqId, numRb, mcs);

    buildCrcData(1);
    m_state = CRC_ERR_IND_SENT;
    m_stsCounter->countRRCSetupComplCrcSent();

    if (m_numHarqTx >= 4) {
    	m_numHarqTx = 0;
    	this->stopT300();
		m_state = RRC_RELEASING;
		LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
    }
}

//// --------------------------------------------
//void PhySendCrcErrorInd::ulHarqResultCallback(UInt16 harqId, BOOL result, UInt8 ueState) {
//    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, result = %d, ueState = %d, harqId = %d, \n",  __func__,
//        m_uniqueId, result, ueState, harqId);
//
//    if (result == FALSE) {
//    	m_numHarqTx++;
//    	m_harqEntity->resetUlHarq();
//    	LOG_DBG(UE_LOGGER_NAME, "[%s], %s, m_numHarqTx = %d\n",  __func__, m_uniqueId, m_numHarqTx);
//    	if (m_numHarqTx > 4) {
//    		m_numHarqTx = 0;
//    		this->stopT300();
//			m_state = WAIT_TERMINATING;
//		    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
//    	}
//    }
//
//}
