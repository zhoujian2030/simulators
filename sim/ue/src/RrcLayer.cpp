/*
 * RrcLayer.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */

#include "RrcLayer.h"
#include "UeTerminal.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif

using namespace ue;

// -------------------------------------
RrcLayer::RrcLayer(UeTerminal* ue) 
: m_ueTerminal(ue)
{

}

// -------------------------------------
RrcLayer::~RrcLayer() {

}

// -------------------------------------
void RrcLayer::handleRxRRCMessage(UInt8* buffer, UInt32 length) {    
	LOG_DBG(UE_LOGGER_NAME, "[%s], length = %d\n", __func__, length);

#ifdef OS_LINUX
    LOG_BUFFER(buffer, length);
#endif

    RrcMsg rrcMsg;
    UInt32 msgType = decode(buffer, length, &rrcMsg);   

    m_ueTerminal->rrcCallback(msgType, &rrcMsg);
}

// -------------------------------------
void RrcLayer::buildIdentityResponse(UInt8* buffer, UInt32& length) {
    // 48 02 22 fd f9 33 37 a2 a0 ea c1 09 20 c6 02 00 00 10 24 a0 
    // UInt8 identityRsp[20] = {
    //     0x48, 0x02, 0x22, 0xfd, 0xf9, 0x33, 0x37, 0xa2, 0xa0, 0xea, 
    //     0xc1, 0x09, 0x20, 0xc6, 0x02, 0x00, 0x00, 0x10, 0x24, 0xa0};

    UInt8 identityRsp[20] = { 0x48, 0x01, 0x60, 0xeA, 0xc1, 0x09, 0x20, 0xC4, 0x0C, 0x20,
                        0x8e, 0xC2, 0x00, 0x00};  
    // UInt8 imsi = ((m_ueTerminal->m_ueId % 10) << 4) | ((m_ueTerminal->m_ueId / 10) >> 4);
    // identityRsp[12] = imsi & >> 3;
    // identityRsp[13] = imsi << 5;

    UInt8 imsiOtect0 = m_ueTerminal->m_ueId % 10;
    UInt8 imsiOtect1 = m_ueTerminal->m_ueId / 10;
    identityRsp[12] = (imsiOtect0 << 1) | ((imsiOtect1 & 0x08) >> 3);
    identityRsp[13] = (imsiOtect1 & 0x07) << 5;

    memcpy(buffer, identityRsp, 20);
    length += 20;
}

// -------------------------------------
UInt32 RrcLayer::decode(UInt8* buffer, UInt32 length, RrcMsg* rrcMsg) {
    // TODO
    // not asn.1 decode, just simple comparing, may decode wrong
    // Identity Request: 0a 00 18 3a a8 08 00
    UInt32 msgType = INVALID_MSG;
    
    UInt32 idx = 0; 
    UInt8 rrcMsgType = buffer[idx] >> 3;  // 0a
    if (rrcMsgType == 0x01) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], Recv DL DT message\n", __func__);

        UInt8 criticalExt = buffer[idx++] & 0x01;  // 0a
        if (criticalExt == 0) {
            idx++;

            // parse from 3rd byte, 18 3a a8 08 00
            UInt8 firstByte = buffer[idx++];
            UInt8 secHeaderType = ((firstByte & 0x07) << 1) | (buffer[idx] >> 7);  // 18 3a
//            LOG_DBG(UE_LOGGER_NAME, "[%s], buffer[%d] = %02x\n", __func__, idx, buffer[idx]);
            if (secHeaderType == 0x00) {
                // not security protected
                UInt8 protocolDiscriminator = (buffer[idx] >> 3) & 0x0f;  // 3a
                if (protocolDiscriminator == 0x07) {
                    firstByte = buffer[idx++];
                    UInt8 nasMsgType = ((firstByte & 0x07) << 5) | ((buffer[idx++] >> 3) & 0x1f);  // 3a a8
                    switch (nasMsgType) {
                        case 0x55:
                        {
                            msgType = IDENTITY_REQUEST;
                            rrcMsg->identityReq.identityType = (buffer[idx] >> 3) & 0x0f;
                            break;
                        }

                        case 0x44:
                        {
                            msgType = ATTACH_REJECT;
                            break;
                        }

                        default:
                        {
                            LOG_INFO(UE_LOGGER_NAME, "[%s], other NAS message: 0x%02x\n", __func__, nasMsgType);
                            break;
                        }
                    }
                }
            }
        }
    } else if (rrcMsgType == 0x05) {
    	LOG_DBG(UE_LOGGER_NAME, "[%s], Recv RRC Release message\n", __func__);
        msgType = RRC_RELEASE;
    } else {
    	LOG_INFO(UE_LOGGER_NAME, "[%s], other message: 0x%02x\n", __func__, rrcMsgType);
    }

    return msgType;
}
