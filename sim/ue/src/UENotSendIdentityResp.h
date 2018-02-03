/*
 * UENotSendIdentityResp.h
 *
 *  Created on: Jan 30, 2018
 *      Author: zhoujian
 */

#ifndef UENOTSENDIDENTITYRESP_H_
#define UENOTSENDIDENTITYRESP_H_

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class UENotSendIdentityResp: public UeTerminal {
	public:
		UENotSendIdentityResp(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~UENotSendIdentityResp();

	protected:
		virtual void rrcCallback(UInt32 msgType, RrcMsg* msg);
	};

}

#endif /* UENOTSENDIDENTITYRESP_H_ */
