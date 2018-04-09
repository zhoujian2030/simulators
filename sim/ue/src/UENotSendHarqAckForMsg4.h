/*
 * UENotSendHarqAckForMsg4.h
 *
 *  Created on: Apr 2, 2018
 *      Author: J.ZH
 */

#ifndef UENOTSENDHARQACKFORMSG4_H_
#define UENOTSENDHARQACKFORMSG4_H_

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class UENotSendHarqAckForMsg4 : public UeTerminal {
	public:
		UENotSendHarqAckForMsg4(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~UENotSendHarqAckForMsg4();

	protected:
		virtual void dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result);
	};

}


#endif /* UENOTSENDHARQACKFORMSG4_H_ */
