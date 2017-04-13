/*
 * PdcpLayer.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */

#include "PdcpLayer.h"
#include "RrcLayer.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif

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
	LOG_DBG(UE_LOGGER_NAME, "[%s], length = %d\n", __func__, length);

#ifdef OS_LINUX
    LOG_BUFFER(buffer, length);
#endif

    // remove one byte PDCP head for SRB
    m_rrcLayer->handleRxRRCMessage(&buffer[1], length-1);
}

// -------------------------------------
void PdcpLayer::buildSrb1Header(UInt8* buffer, UInt32& length) {
    memmove(buffer+1, buffer, length);
    buffer[0] = 0x01;
    length += 1;
}
