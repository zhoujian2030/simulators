/*
 * UENotSendRlcAck.h
 *
 *  Created on: 2017-3-16
 *      Author: j.zhou
 */

#ifndef UENOTSENDRLCACK_H_
#define UENOTSENDRLCACK_H_

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class UENotSendRlcAck : public UeTerminal {
	public:
		UENotSendRlcAck(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~UENotSendRlcAck();

		virtual void resetChild();

	protected:
//		virtual void dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result);

        virtual void rrcCallback(UInt32 msgType, RrcMsg* msg);
        virtual void rlcCallback(UInt8* statusPdu, UInt32 length);

        UInt8 m_numIdentityReqRecvd;
        UInt8 m_numAttachRejRecvd;
        UInt8 m_numRRCRelRecvd;

	};

}


#endif /* UENOTSENDRLCACK_H_ */
