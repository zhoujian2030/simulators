/*
 * UENotSendRrcSetupComplete.h
 *
 *  Created on: 2017-3-15
 *      Author: j.zhou
 */

#ifndef UENOTSENTRRCSETUPCOMPLETE_H_
#define UENOTSENTRRCSETUPCOMPLETE_H_

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class UENotSendRrcSetupComplete : public UeTerminal {
	public:
		UENotSendRrcSetupComplete(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~UENotSendRrcSetupComplete();

	protected:
		virtual void dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result);
	};

}


#endif /* UENOTSENTRRCSETUPCOMPLETE_H_ */
