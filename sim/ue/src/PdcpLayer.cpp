/*
 * PdcpLayer.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */

#include "PdcpLayer.h"
#include "RrcLayer.h"
#include "CLogger.h"

using namespace ue;

// -------------------------------------
PdcpLayer::PdcpLayer(RrcLayer* rrcLayer) 
: m_rrcLayer(rrcLayer)
{

}

// -------------------------------------
PdcpLayer::~PdcpLayer() {

}

// -------------------------------------
void PdcpLayer::handleRxSrb(UInt8* buffer, UInt32 length) {
    LOG_DEBUG(UE_LOGGER_NAME, "length = %d\n", length);

    for (UInt16 j=0; j<length; j++) {
        printf("%02x ", buffer[j]);
    }
    printf("\n"); 

    // remove one byte PDCP head for SRB
    m_rrcLayer->handleRxRRCMessage(&buffer[1], length-1);
}

// -------------------------------------
void PdcpLayer::buildSrb1Header(UInt8* buffer, UInt32& length) {
    memmove(buffer+1, buffer, length);
    buffer[0] = 0x01;
    length += 1;
}