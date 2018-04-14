/*
 * RlcLayer.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */

#include "RlcLayer.h"
#include "PdcpLayer.h"
#include "UeTerminal.h"
#include <vector>

using namespace ue;
using namespace std;

// -------------------------------------
RlcLayer::RlcLayer(UeTerminal* ue, PdcpLayer* pdcpLayer) 
: m_ue(ue), m_pdcpLayer(pdcpLayer)
{
    m_nodePool = new NodePool(MAX_RLC_AMD_SDU_SEG_NUM);
    m_firstSeg = 0;

    m_rrcSetupConfig.tPollRetransmit = 80;
    m_rrcSetupConfig.pollPDU = 128;
    m_rrcSetupConfig.pollByte = 125;
    m_rrcSetupConfig.maxRetxThreshold = 16;
    m_rrcSetupConfig.tReordering = 80;
    m_rrcSetupConfig.tStatusProhibit = 15;

    // RRC setup complete id the first AM PDU sent with sn = 0,
    // which is not built by RLcLayer currently
    m_sn = 1;
    m_ackSn = 0;
    m_timerValue = -1;
}

// -------------------------------------
RlcLayer::~RlcLayer() {

}

// -------------------------------------
void RlcLayer::reset() { 
    LOG_TRACE(UE_LOGGER_NAME, "[%s], Entry\n", __func__);
    m_sn = 1;
    m_numSegRecvd = 0;
    freeAllSegments();
    m_firstSeg = 0;
}

// -------------------------------------
void RlcLayer::handleRxAMDPdu(UInt8* buffer, UInt32 length) {    
	LOG_DBG(UE_LOGGER_NAME, "[%s], length = %d\n", __func__, length);

#ifdef OS_LINUX
    LOG_BUFFER(buffer, length); 
#endif

    UInt32 idx = 0;
    m_amdHeader.dc = (buffer[idx] >> 7) & 0x01;

    if (m_amdHeader.dc) {
        /* Get Re-segmentation Flag (RF) field */
        m_amdHeader.rf =  (buffer[idx] >> 6 ) & 0x01U;
        /* Get Polling bit (P) field */
        m_amdHeader.p  =  (buffer[idx] >> 5 ) & 0x01U;
        /* Get Framing Info (FI) field */
        m_amdHeader.fi =  (buffer[idx] >> 3 ) & 0x03U;
        /* Get Extension bit (E) field */
        m_amdHeader.e  =  (buffer[idx] >> 2 ) & 0x01U;
        /* Get Sequence Number (SN) field */
        m_amdHeader.sn = ( ( (buffer[idx] & 0x03U) << 8 )| (buffer[idx + 1] & 0xFFU) );

        /*Decode AMD PDU Segment*/    
        if ( m_amdHeader.rf )
        {
            /* Get Last Segment Flag (LSF) field */
            m_amdHeader.lsf =  (buffer[idx + 2] >> 7 ) & 0x01U;
            /* Get Segment Offset (SO) field */
           /* m_amdHeader.so  = ( ((buffer[idx + 2] << 8) & 0x3FU ) 
                    | (buffer[idx + 3] & 0xFFU ));*/
            m_amdHeader.so  = ( ((UInt16)(buffer[idx + 2] & 0x7FU ) << 8)
                               | (buffer[idx + 3] & 0xFFU )); //abhishek
        }

//        LOG_DBG(UE_LOGGER_NAME, "[%s], dc = %d\n", __func__, m_amdHeader.dc);
//        LOG_DBG(UE_LOGGER_NAME, "[%s], rf = %d\n", __func__, m_amdHeader.rf);
//        LOG_DBG(UE_LOGGER_NAME, "[%s], p = %d\n", __func__, m_amdHeader.p);
//        LOG_DBG(UE_LOGGER_NAME, "[%s], fi = %d\n", __func__, m_amdHeader.fi);
//        LOG_DBG(UE_LOGGER_NAME, "[%s], e = %d\n", __func__, m_amdHeader.e);
//        LOG_DBG(UE_LOGGER_NAME, "[%s], sn = %d\n", __func__, m_amdHeader.sn);
//        LOG_DBG(UE_LOGGER_NAME, "[%s], lsf = %d\n", __func__, m_amdHeader.lsf);
//        LOG_DBG(UE_LOGGER_NAME, "[%s], so = %d\n", __func__, m_amdHeader.so);

        reassembleAMDPdu(&m_amdHeader, &buffer[2], length-2);
    } else {
    	//
    	UInt8 cpt = (buffer[idx] >> 4) & 0x07;
    	if (cpt == 0) {
    		m_ackSn = ((buffer[idx] & 0x0f) << 6) | ((buffer[idx+1] >> 2) &0x3f);
        	LOG_INFO(UE_LOGGER_NAME, "[%s], receive rlc status PDU, m_ackSn = %d, m_sn = %d, SDU length = %d, ueId = %02x\n", __func__, m_ackSn, m_sn, length, m_ue->getUeId());
        	//TODO
        	stopTimer();
    	}
    }
}

// -------------------------------------
void RlcLayer::buildRlcAMDHeader(UInt8* buffer, UInt32& length) {
    // only support single segment SDU
    memmove(buffer+2, buffer, length);
#if 1
    buffer[0] = 0x80 | 0x20 | ((m_sn >> 8) & 0x03); // dc = 1, p = 1, need status report
#else
    buffer[0] = 0x80 | 0x00 | ((m_sn >> 8) & 0x03); // dc = 1, p = 0, no need status report
#endif
    buffer[1] = m_sn & 0xff;
    length += 2;

    m_sn = (++m_sn)%1024;

    startTimer();
}

// -------------------------------------
void RlcLayer::startTimer() {
	if (m_timerValue <= 0) {
		m_timerValue = 60;
	} else {
		LOG_WARN(UE_LOGGER_NAME, "[%s], not receive RLC ACK for last RLC PDU, m_sn = %d, m_ackSn = %d, ueId = %02x\n", __func__, m_sn, m_ackSn, m_ue->getUeId());
	}
}

// -------------------------------------
BOOL RlcLayer::processTimer() {
	if (m_timerValue < 0) {
		return FALSE;
	}

	if (m_timerValue == 0) {
		LOG_WARN(UE_LOGGER_NAME, "[%s], receive RLC ACK timeout, m_sn = %d, m_ackSn = %d, ueId = %02x\n", __func__, m_sn, m_ackSn, m_ue->getUeId());
		return TRUE;
	}

	m_timerValue--;
	return FALSE;
}


// -------------------------------------
void RlcLayer::stopTimer() {
	m_timerValue = -1;
}

// -------------------------------------
void RlcLayer::reassembleAMDPdu(AmdHeader* header, UInt8* buffer, UInt32 length) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], fi = %d, sn = %d, p = %d, e = %d, length = %d\n", __func__,
        header->fi, header->sn, header->p, header->e, length);
    
    if (header->fi == FI_00_SINGLE_SEG) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], receive a single SDU with only one segment\n", __func__);
        if (header->e) {
            UInt32 sduLength = 0;
            handleExtField(sduLength, buffer, length);
            if (sduLength > 0) {
                m_pdcpLayer->handleRxSrb(m_rlcSdu, sduLength);
            }
        } else {
            m_pdcpLayer->handleRxSrb(buffer, length);
        }
    } else if (header->fi == FI_01_FIRST_SEG) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], receive the first segment\n", __func__);
        if (length > MAX_RLC_AMD_SDU_SEG_LENGTH) {            
            LOG_ERROR(UE_LOGGER_NAME, "[%s], length exceeds max defined value %d\n", __func__, MAX_RLC_AMD_SDU_SEG_LENGTH);
            return;
        }
        m_firstSeg = m_nodePool->getNode();
        if (m_firstSeg == 0) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], No available node\n", __func__);
            return;
        }
        m_firstSeg->length = length;
        m_firstSeg->sn = header->sn;
        m_firstSeg->e  = header->e;
        memcpy(m_firstSeg->buffer, buffer, length);
        m_firstSeg->next = m_firstSeg;
        m_firstSeg->prev = m_firstSeg;
        m_numSegRecvd = 1;
    } else if (header->fi == FI_02_LAST_SEG) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], receive the last segment\n", __func__);

        if (m_firstSeg == 0) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], invalid segment, drop it\n", __func__);
            return;
        }
        
        Node* prevSeg = m_firstSeg->prev;
        UInt16 expectSn = (prevSeg->sn + 1) % 1023;
        if (expectSn != header->sn) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], invalid sn, dorp this whole RLC SDU\n", __func__);
            freeAllSegments();
            return;
        }
        if (m_firstSeg->sn + m_numSegRecvd < header->sn) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], missing segment, dorp this whole RLC SDU\n", __func__);
            freeAllSegments();
            return;
        }

        Node* node = m_nodePool->getNode();
        if (node == 0) {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], No available node\n", __func__);
            return;
        }
        node->length = length;
        node->sn = header->sn;
        node->e = header->e;
        memcpy(node->buffer, buffer, length);
        prevSeg->next = node;
        node->prev = prevSeg;
        node->next = m_firstSeg;
        m_firstSeg->prev = node;

        // parse all segments
        UInt32 sduLength = 0;
        while (1) {
            if (m_firstSeg->e) {
                handleExtField(sduLength, m_firstSeg->buffer, m_firstSeg->length);
            } else {
                memcpy(m_rlcSdu + sduLength, m_firstSeg->buffer, m_firstSeg->length);
                sduLength += m_firstSeg->length;
            }            
            
            if (m_firstSeg->next == m_firstSeg) {
            	LOG_INFO(UE_LOGGER_NAME, "[%s], last segment!\n", __func__);
                if (sduLength > 0) {
                    m_pdcpLayer->handleRxSrb(m_rlcSdu, sduLength);
                }
                m_nodePool->freeNode(m_firstSeg);
                m_firstSeg = 0;
                break;
            }

            Node* node = m_firstSeg;
            m_firstSeg = node->next;
            m_firstSeg->prev = node->prev;
            node->prev->next = m_firstSeg;
            m_nodePool->freeNode(node);
        }
    } else {
    	LOG_INFO(UE_LOGGER_NAME, "[%s], receive the middle segment, TODO\n", __func__);
        // middle segment received, need to assemble
        // TODO
    }

    if (header->p == 1) {
        // currently only support 2 bytes status PDU (E1 = 0)
        UInt8 statusPdu[2];
        UInt16 ackSn = (header->sn + 1)%1024;
        statusPdu[0] = (ackSn >> 6) & 0x0f;
        statusPdu[1] = (ackSn & 0x3f) << 2;
        LOG_INFO(UE_LOGGER_NAME, "[%s], need to send RLC ACK later, ackSn = %d\n", __func__, ackSn);
        m_ue->rlcCallback(statusPdu, 2);

        // TODO support E1 = 1
    }
}

// -------------------------------------
void RlcLayer::handleExtField(UInt32& sduLength, UInt8* buffer, UInt32 length) {    
    LOG_TRACE(UE_LOGGER_NAME, "[%s], sduLength = %d, length = %d\n", __func__, sduLength, length);
    UInt8 idx = 0;
    UInt8 e = 0;
    UInt16 li = 0;
    vector<UInt16> liVect;

    while (1) {
        li = ((buffer[idx] & 0x7f) << 4) | ((buffer[idx + 1] >> 4) & 0x0f);
        liVect.push_back(li);
        e = (buffer[idx] >> 7) & 0x01;
        idx++;
        if (e == 1) {
            li = ((buffer[idx] & 0x07) << 8) | (buffer[idx + 1]);
            liVect.push_back(li);
            e = (buffer[idx] >> 3) & 0x01;
            idx += 2;
            if (e == 0) {
                LOG_DBG(UE_LOGGER_NAME, "[%s], parse end 2\n", __func__);
                break;
            }
        } else {
            idx++;
            LOG_DBG(UE_LOGGER_NAME, "[%s], parse end 1\n", __func__);
            break;
        }
    }

    UInt32 size = liVect.size();

    for (UInt32 i=0; i<size; i++) {
        li = liVect[i];            
        LOG_DBG(UE_LOGGER_NAME, "[%s], li = %d\n", __func__, li);
        if (li + idx <= length) {
            if (i == 0 && sduLength > 0) {
                // pre RLC SDU segment is saved in m_rlcSdu
                memcpy(m_rlcSdu + sduLength, &buffer[idx], li);
                m_pdcpLayer->handleRxSrb(m_rlcSdu, li+sduLength);
                sduLength = 0;
            } else {
                if ((idx + li) == length) {
                    // the last li 
                    sduLength = li;
                    memcpy(m_rlcSdu, &buffer[idx], li);
                    idx = length;
                    break;
                } else {
                    m_pdcpLayer->handleRxSrb(&buffer[idx], li);
                }
            }
            idx += li;
        } else {
            break;
        }
    }

    if (idx < (length - 1)) {
        li = length - idx;
        sduLength = li;
        memcpy(m_rlcSdu, &buffer[idx], li);
    }

    liVect.clear();    
}
