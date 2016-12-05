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
    m_nodePool = new NodePool(MAC_RLC_AMD_SDU_SEG_NUM);
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
}

// -------------------------------------
RlcLayer::~RlcLayer() {

}

// -------------------------------------
void RlcLayer::reset() { 
    LOG_DEBUG(UE_LOGGER_NAME, "Entry\n");
    m_sn = 1;
    m_numSegRecvd = 0;
    freeAllSegments();
    m_firstSeg = 0;
}

// -------------------------------------
void RlcLayer::handleRxAMDPdu(UInt8* buffer, UInt32 length) {    
    LOG_DEBUG(UE_LOGGER_NAME, "length = %d\n", length);

    for (UInt16 j=0; j<length; j++) {
        printf("%02x ", buffer[j]);
    }
    printf("\n"); 

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

        LOG_DEBUG(UE_LOGGER_NAME, "dc = %d\n", m_amdHeader.dc);
        LOG_DEBUG(UE_LOGGER_NAME, "rf = %d\n", m_amdHeader.rf);
        LOG_DEBUG(UE_LOGGER_NAME, "p = %d\n", m_amdHeader.p);
        LOG_DEBUG(UE_LOGGER_NAME, "fi = %d\n", m_amdHeader.fi);
        LOG_DEBUG(UE_LOGGER_NAME, "e = %d\n", m_amdHeader.e);
        LOG_DEBUG(UE_LOGGER_NAME, "sn = %d\n", m_amdHeader.sn);
        LOG_DEBUG(UE_LOGGER_NAME, "lsf = %d\n", m_amdHeader.lsf);
        LOG_DEBUG(UE_LOGGER_NAME, "so = %d\n", m_amdHeader.so);

        reassembleAMDPdu(&m_amdHeader, &buffer[2], length-2);
    }     
}

// -------------------------------------
void RlcLayer::buildRlcAMDHeader(UInt8* buffer, UInt32& length) {
    // only support single segment SDU
    memmove(buffer+2, buffer, length);
    buffer[0] = 0x80 | 0x20 | ((m_sn >> 8) & 0x03); // dc = 1, p = 1, need status report
    buffer[1] = m_sn & 0xff;
    length += 2;

    m_sn = (++m_sn)%1024;
}

// -------------------------------------
void RlcLayer::reassembleAMDPdu(AmdHeader* header, UInt8* buffer, UInt32 length) {
    LOG_DEBUG(UE_LOGGER_NAME, "fi = %d, sn = %d, p = %d, e = %d, length = %d\n", 
        header->fi, header->sn, header->p, header->e, length);
    
    if (header->fi == FI_00_SINGLE_SEG) {
        LOG_DEBUG(UE_LOGGER_NAME,"receive a single SDU with only one segment\n");
        handleExtentionField(header->e, buffer, length);
    } else if (header->fi == FI_01_FIRST_SEG) {
        LOG_DEBUG(UE_LOGGER_NAME,"receive the first segment\n");
        if (length > MAX_RLC_AMD_SDU_SEG_LENGTH) {            
            LOG_ERROR(UE_LOGGER_NAME, "length exceeds max defined value %d\n", MAX_RLC_AMD_SDU_SEG_LENGTH);
            return;
        }
        m_firstSeg = m_nodePool->getNode();
        if (m_firstSeg == 0) {
            LOG_ERROR(UE_LOGGER_NAME, "No available node\n");
            return;
        }
        m_firstSeg->length = length;
        m_firstSeg->sn = header->sn;
        memcpy(m_firstSeg->buffer, buffer, length);
        m_firstSeg->next = m_firstSeg;
        m_firstSeg->prev = m_firstSeg;
        m_numSegRecvd = 1;
    } else if (header->fi == FI_02_LAST_SEG) {
        LOG_DEBUG(UE_LOGGER_NAME,"receive the last segment\n");
        Node* prevSeg = m_firstSeg->prev;
        UInt16 expectSn = (prevSeg->sn + 1) % 1023;
        if (expectSn != header->sn) {
            LOG_ERROR(UE_LOGGER_NAME, "invalid sn, dorp this whole RLC SDU\n");
            freeAllSegments();
            return;
        }
        if (m_firstSeg->sn + m_numSegRecvd < header->sn) {
            LOG_ERROR(UE_LOGGER_NAME, "missing segment, dorp this whole RLC SDU\n");
            freeAllSegments();
            return;
        }

        if (prevSeg == m_firstSeg) {
            // all previous data is saved in one node
            if ((length + prevSeg->length) <= MAX_RLC_AMD_SDU_SEG_LENGTH) {
                memcpy(prevSeg->buffer + prevSeg->length, buffer, length);
                prevSeg->length += length;
                handleExtentionField(header->e, prevSeg->buffer, prevSeg->length);
            } else {
                memcpy(m_rlcSdu, prevSeg->buffer, prevSeg->length);
                memcpy(m_rlcSdu + prevSeg->length, buffer, length);
                handleExtentionField(header->e, m_rlcSdu, prevSeg->length + length);
            }
            m_nodePool->freeNode(m_firstSeg);
            m_firstSeg = 0;
        } else {
            // more than one node
            UInt32 sduLength = 0;
            while (1) {
                memcpy(m_rlcSdu + length, m_firstSeg->buffer, m_firstSeg->length);
                sduLength += m_firstSeg->length;
                if (m_firstSeg->next = m_firstSeg) {
                    LOG_DEBUG(UE_LOGGER_NAME,"last segment!\n");
                    handleExtentionField(header->e, m_rlcSdu, sduLength);
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
        }
    } else {
        LOG_DEBUG(UE_LOGGER_NAME,"receive the middle segment, TODO\n");
        // middle segment received, need to assemble
        // TODO
    }

    if (header->p == 1) {
        // currently only support 2 bytes status PDU (E1 = 0)
        UInt8 statusPdu[2];
        UInt16 ackSn = (header->sn + 1)%1024;
        statusPdu[0] = (ackSn >> 6) & 0x0f;
        statusPdu[1] = (ackSn & 0x3f) << 2;
        m_ue->rlcCallback(statusPdu, 2);

        // TODO support E1 = 1
    }
}

// -------------------------------------
void RlcLayer::handleExtentionField(UInt8 ext, UInt8* buffer, UInt32 length) {
    LOG_DEBUG(UE_LOGGER_NAME, "ext = %d, length = %d\n", ext, length);

    if (!ext) {
        m_pdcpLayer->handleRxSrb(buffer, length);
    } else {
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
                    LOG_DEBUG(UE_LOGGER_NAME,"parse end 2\n");
                    break;
                }
            } else {
                idx++;
                LOG_DEBUG(UE_LOGGER_NAME,"parse end 1\n");
                break;
            }
        }

        UInt32 size = liVect.size();
        for (UInt32 i=0; i<size; i++) {
            li = liVect[i];            
            LOG_DEBUG(UE_LOGGER_NAME, "li = %d\n", li);
            if (li + idx <= length) {
                m_pdcpLayer->handleRxSrb(&buffer[idx], li);
                idx += li;
            } else {
                break;
            }
        }

        if (idx < (length - 1)) {
            li = length - idx;
            m_pdcpLayer->handleRxSrb(&buffer[idx], li);
        }

        liVect.clear();
    }
}