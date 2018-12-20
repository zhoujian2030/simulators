/*
 * TestForcedUlGrant.h
 *
 *  Created on: Sep 17, 2018
 *      Author: J.ZH
 */

#ifndef TESTFORCEDULGRANT_H_
#define TESTFORCEDULGRANT_H_

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class TestForcedUlGrant : public UeTerminal {
	public:
		TestForcedUlGrant(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~TestForcedUlGrant();

	protected:
		virtual void startT300();
		virtual void dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result);
		virtual void rrcCallback(UInt32 msgType, RrcMsg* msg);
		virtual void handleDci0Pdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlDCIPduInfo_st* pDci0Pdu);
	};

    // -------------------------------------------------------
    inline void TestForcedUlGrant::startT300() {
        m_t300Value = 60000;
    }
}


#endif /* TESTFORCEDULGRANT_H_ */
