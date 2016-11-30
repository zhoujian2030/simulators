/*
 * RlcLayer.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */

#include "RlcLayer.h"
#include "PdcpLayer.h"
#include "CLogger.h"

using namespace ue;

// -------------------------------------
RlcLayer::RlcLayer(PdcpLayer* pdcpLayer) 
: m_pdcpLayer(pdcpLayer)
{
    // RRC setup complete id the first AM PDU sent with sn = 0,
    // which is not built by RLcLayer currently
    m_sn = 1;
}

// -------------------------------------
RlcLayer::~RlcLayer() {

}

// -------------------------------------
void RlcLayer::handleRxAMDPdu(UInt8* buffer, UInt32 length) {
    UInt8 dc = (buffer[0] >> 7) & 0x01;
    
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

        m_pdcpLayer->handleRxSrb(&buffer[2], length-2);
    }     
}

// -------------------------------------
void RlcLayer::buildRlcAMDHeader(UInt8* buffer, UInt32& length) {
    memmove(buffer+2, buffer, length);
    buffer[0] = 0x80 | ((m_sn >> 8) & 0x03);
    buffer[1] = m_sn & 0xff;
    length += 2;

    m_sn++;
}