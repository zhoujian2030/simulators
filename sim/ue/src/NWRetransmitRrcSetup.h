/*
 * NWRetransmitRrcSetup.h
 *
 *  Created on: 2017-3-15
 *      Author: j.zhou
 */

#ifndef NW_RETRANSMIT_RRCSETUP_H
#define NW_RETRANSMIT_RRCSETUP_H

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class NWRetransmitRrcSetup : public UeTerminal {
	public:
		NWRetransmitRrcSetup(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~NWRetransmitRrcSetup();

	protected:
		virtual void allocateDlHarqCallback(UInt16 harqProcessNum, BOOL result);
		virtual void dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result);

		UInt16 m_numRRCSetupRetrans;
	};

}


#endif /* UENOTSENDHARQACK_H_ */
