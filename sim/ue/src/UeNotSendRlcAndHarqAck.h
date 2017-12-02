/*
 * UeNotSendRlcAndHarqAck.h
 *
 *  Created on: Nov 23, 2017
 *      Author: J.ZH
 */

#ifndef UENOTSENDRLCANDHARQACK_H_
#define UENOTSENDRLCANDHARQACK_H_

#include "UENotSendRlcAck.h"

namespace ue {

class UeNotSendRlcAndHarqAck: public ue::UENotSendRlcAck {
public:
	UeNotSendRlcAndHarqAck(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
	virtual ~UeNotSendRlcAndHarqAck();

protected:
	virtual void dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result);
};

} /* namespace ue */
#endif /* UENOTSENDRLCANDHARQACK_H_ */
