/*
 * UESuspending.h
 *
 *  Created on: 2017-3-16
 *      Author: j.zhou
 */

#ifndef UESUSPENDING_H_
#define UESUSPENDING_H_

#include "UeTerminal.h"

// @Description
// UE suspend after RRC connected (after receive HARQ ACK for RRC SETUP Complete).
// Attach reject and RRC release later will be drop, which lead to network MAC HARQ retransmit
// and RLC ARQ retransmit.
// RRC will delete UE context after timeout, UE will send delete response to MAC.

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class UESuspending : public UeTerminal {
	public:
		UESuspending(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~UESuspending();

	protected:
		virtual void ulHarqResultCallback(UInt16 harqId, BOOL result, UInt8 ueState);
		virtual void rrcCallback(UInt32 msgType, RrcMsg* msg);
	};

}


#endif /* UESUSPENDING_H_ */
