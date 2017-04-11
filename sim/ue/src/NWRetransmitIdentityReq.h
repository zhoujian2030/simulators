/*
 * NWRetransmitIdentityReq.h
 *
 *  Created on: 2017-3-15
 *      Author: j.zhou
 */

#ifndef NWRETRANSMITIDENTITYREQ_H_
#define NWRETRANSMITIDENTITYREQ_H_

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class NWRetransmitIdentityReq : public UeTerminal {
	public:
		NWRetransmitIdentityReq(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~NWRetransmitIdentityReq();

		virtual void resetChild();

	protected:
		virtual void dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result);

        virtual void rrcCallback(UInt32 msgType, RrcMsg* msg);
        virtual void rlcCallback(UInt8* statusPdu, UInt32 length);

        UInt16 m_numIdentityReqRetrans;
	};

}



#endif /* NWRETRANSMITIDENTITYREQ_H_ */
