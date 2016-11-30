/*
 * PdcpLayer.h
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */
 
#ifndef PDCP_LAYER_H
#define PDCP_LAYER_H

#include "UeBaseType.h"

namespace ue {

    class RrcLayer;

    class PdcpLayer {
    public:
        PdcpLayer(RrcLayer* rrcLayer);
        ~PdcpLayer();

        void handleRxSrb(UInt8* buffer, UInt32 length);
        void buildSrb1Header(UInt8* buffer, UInt32& length);

    private:
        RrcLayer* m_rrcLayer;
    };
}

#endif
