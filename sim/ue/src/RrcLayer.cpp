/*
 * RrcLayer.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */

#include "RrcLayer.h"
#include "UeTerminal.h"
#include "CLogger.h"

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
    LOG_DEBUG(UE_LOGGER_NAME, "length = %d\n", length);

    for (UInt16 j=0; j<length; j++) {
        printf("%02x ", buffer[j]);
    }
    printf("\n"); 

    RrcMsg rrcMsg;
    UInt32 msgType = decode(buffer, length, &rrcMsg);   

    m_ueTerminal->rrcCallback(msgType, &rrcMsg);
}

// -------------------------------------
void RrcLayer::buildIdentityResponse(UInt8* buffer, UInt32& length) {
    // 48 02 22 fd f9 33 37 a2 a0 ea c1 09 20 c6 02 00 00 10 24 a0 
    UInt8 identityRsp[20] = {
        0x48, 0x02, 0x22, 0xfd, 0xf9, 0x33, 0x37, 0xa2, 0xa0, 0xea, 
        0xc1, 0x09, 0x20, 0xc6, 0x02, 0x00, 0x00, 0x10, 0x24, 0xa0};

    memcpy(buffer, identityRsp, 20);
    length = 20;
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
        LOG_DEBUG(UE_LOGGER_NAME, "Recv DL DT message\n");

        UInt8 criticalExt = buffer[idx++] & 0x01;  // 0a
        if (criticalExt == 0) {
            idx++;

            // parse from 3rd byte, 18 3a a8 08 00
            UInt8 firstByte = buffer[idx++];
            UInt8 secHeaderType = ((firstByte & 0x07) << 1) | (buffer[idx] >> 7);  // 18 3a
            LOG_DEBUG(UE_LOGGER_NAME, "buffer[%d] = %02x\n", idx, buffer[idx]);
            if (secHeaderType == 0x00) {
                // not security protected
                UInt8 protocolDiscriminator = (buffer[idx] >> 3) & 0x0f;  // 3a
                if (protocolDiscriminator == 0x07) {
                    firstByte = buffer[idx++];
                    UInt8 nasMsgType = ((firstByte & 0x07) << 5) | ((buffer[idx++] >> 3) & 0x1f);  // 3a a8
                    if (nasMsgType == 0x55) {
                        msgType = IDENTITY_REQUEST;
                        rrcMsg->identityReq.identityType = (buffer[idx] >> 3) & 0x0f;
                    } else {
                        LOG_DEBUG(UE_LOGGER_NAME, "other NAS message: 0x%02x\n", nasMsgType);
                    }
                }
            }
        }
    } else if (rrcMsgType == 0x05) {
        LOG_DEBUG(UE_LOGGER_NAME, "Recv RRC Release message\n");
        msgType = RRC_RELEASE;
    } else {
        LOG_DEBUG(UE_LOGGER_NAME, "other message: 0x%02x\n", rrcMsgType);
    }

    return msgType;
}