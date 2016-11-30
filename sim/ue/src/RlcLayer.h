/*
 * RlcLayer.h
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */
 
#ifndef RLC_LAYER_H
#define RLC_LAYER_H

#include "UeBaseType.h"

namespace ue {

    class PdcpLayer;

    class RlcLayer {
    public:
        RlcLayer(PdcpLayer* pdcpLayer);
        ~RlcLayer();

        typedef struct {
            UInt8  dc;
            UInt8  rf;
            UInt8  p;
            UInt8  fi;
            UInt8  e;
            UInt8  lsf;
            UInt16  sn;
            UInt16 so;
            UInt16 soEnd;
        } AmdHeader;

        void handleRxAMDPdu(UInt8* buffer, UInt32 length);
        void buildRlcAMDHeader(UInt8* buffer, UInt32& length);

    private:
        PdcpLayer* m_pdcpLayer;
        AmdHeader m_amdHeader;

        UInt16 m_sn;
    };
}

#endif
