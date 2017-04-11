/*
 * UESendRrcReestablishmentReq.h
 *
 *  Created on: 2017-4-8
 *      Author: j.zhou
 */

#ifndef UESENDRRCREESTABLISHMENTREQ_H_
#define UESENDRRCREESTABLISHMENTREQ_H_

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class UESendRrcReestablishmentReq : public UeTerminal {
	public:
		UESendRrcReestablishmentReq(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~UESendRrcReestablishmentReq();

	protected:
		virtual void buildMsg3Data();
		virtual BOOL parseContentionResolutionPdu(UInt8* data, UInt32 pduLen);
		virtual void handleCCCHMsg(UInt16 rrcMsgType);
	};

}


#endif /* UESENDRRCREESTABLISHMENTREQ_H_ */
