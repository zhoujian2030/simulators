/*
 * PhySendCrcErrorInd.h
 *
 *  Created on: Sep 4, 2018
 *      Author: J.ZH
 */

#ifndef PHY_SEND_CRC_ERROR_IND_H
#define PHY_SEND_CRC_ERROR_IND_H

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class PhySendCrcErrorInd : public UeTerminal {
	public:
		PhySendCrcErrorInd(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~PhySendCrcErrorInd();

	protected:
		virtual void handleDci0Pdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlDCIPduInfo_st* pDci0Pdu);
		virtual void ulHarqSendCallback(UInt16 harqId, UInt8 numRb, UInt8 mcs, UInt8& ueState);
//		virtual void ulHarqResultCallback(UInt16 harqId, BOOL result, UInt8 ueState);

	private:
		int m_numHarqTx;
	};

}


#endif /* PHY_SEND_CRC_ERROR_IND_H */
