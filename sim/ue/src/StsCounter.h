/*
 * StsCounter.h
 *
 *  Created on: Nov 08, 2016
 *      Author: j.zhou
 */

#ifndef STS_COUNTER_H
#define STS_COUNTER_H

#include "UeBaseType.h"

namespace ue {

    class StsCounter {
    public:
        ~StsCounter();

        static StsCounter* getInstance();

        void show();

        void countTestSuccess();
        void countTestFailure();

        void countRachSent();
        void countRachTimeout();
        void countRarRecvd();
        void countRarInvalid();
        void countMsg3ULCfgRecvd();
        void countMsg3Sent();
        void countMsg3CrcSent();
        void countContentionResolutionRecvd();
        void countContentionResolutionInvalid();
        void countContentionResolutionTimeout();
        void countHarqAckSent();
        void countRRCSetupRecvd();
        void countRRCSetupInvalid();
        void countSRSent();
        void countSRTimeout();
        void countRRCSetupComplDCI0Recvd();
        void countRRCSetupComplULCfgRecvd();
        void countRRCSetupComplSent();
        void countRRCSetupComplCrcSent();
        void countRRCSetupComplHarqTimeout();
        void countHarqTimeout();
        void countHarqAckRecvd();
        void countHarqNAckRecvd();
        void countIdentityRequestRecvd();
        void countIdentityRequestTimeout();
        void countIdentityResponseSent(); 
        void countAttachRejectRecvd();
        void countRRCRelRecvd();
        void countT300Timeout();
        void countNonConsecutiveSfnSf();
        void countTACmdRecvd();
        void countForceULGrantRecvd();
        void countBSRTimeout();
        void countRRCSetupTimeout();
        void countInvalidState();
        void countAllocDlHarqFailure();
        void countSuccRejTest();
        void countRRCRestabRej();
        void countRRCConnRej();
        void countRRCSetupRetransmit();
        void countIdentityReqRetransmit();

        UInt32 getNumHarqTimeout();
        UInt32 getNumInvalidState();

    private:
        StsCounter();

        static StsCounter* m_theInstance;

        BOOL m_isChanged;

        UInt32 m_succTest;
        UInt32 m_failureTest;

        UInt32 m_rachSent;
        UInt32 m_rachTimeout;

        UInt32 m_rarRecvd;
        UInt32 m_rarInvalid;

        UInt32 m_msg3UlCfgRecvd;
        UInt32 m_msg3Sent;
        UInt32 m_msg3CrcSent;

        UInt32 m_contentionResolutionRecvd;
        UInt32 m_contentionResolutionInvalid;
        UInt32 m_contentionResolutionTimeout;

        UInt32 m_harqAckSent;

        UInt32 m_rrcSetupRecvd;
        UInt32 m_rrcSetupInvalid;

        UInt32 m_srSent;
        UInt32 m_srTimeout;

        UInt32 m_rrcSetupComplDCI0Recvd;
        UInt32 m_rrcSetupComplUlCfgRecvd;
        UInt32 m_rrcSetupComplSent;
        UInt32 m_rrcSetupComplCrcSent;
        UInt32 m_rrcSetupComplHarqTimeout;

        UInt32 m_harqTimeout;
        UInt32 m_harqAckRecvd;
        UInt32 m_harqNAckRecvd;

        UInt32 m_identityReqRecvd;
        UInt32 m_identityReqTimeout;
        UInt32 m_identityRespSent;
        UInt32 m_attachRejectRecvd;
        UInt32 m_rrcReleaseRecvd;

        UInt32 m_t300Timeout;

        UInt32 m_nonConsecutiveSfnSf;

        UInt32 m_numTACmdRecvd;
        UInt32 m_numForceULGrantRecvd;

        UInt32 m_numBsrTimeout;

        UInt32 m_numRRCSetupTimeout;

        UInt32 m_invalidState;

        UInt32 m_allocDlHarqFailure;

        UInt32 m_numRRCReestabRej;

        UInt32 m_numRRCConnRej;

        UInt32 m_numSuccRejTest;

        UInt32 m_rrcSetupRetransmit;

        UInt32 m_identityReqRetransmit;
    };

    // -----------------------------------------
    inline UInt32 StsCounter::getNumHarqTimeout() {
    	return this->m_harqTimeout;
    }

    // -----------------------------------------
    inline UInt32 StsCounter::getNumInvalidState() {
    	return this->m_invalidState;
    }

    // -----------------------------------------
	inline void StsCounter::countIdentityReqRetransmit() {
		m_identityReqRetransmit++;
		m_isChanged = TRUE;
	}

    // -----------------------------------------
	inline void StsCounter::countRRCSetupRetransmit() {
		m_rrcSetupRetransmit++;
		m_isChanged = TRUE;
	}

    // -----------------------------------------
    inline void StsCounter::countSuccRejTest() {
    	m_numSuccRejTest++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCRestabRej() {
    	m_numRRCReestabRej++;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCConnRej() {
    	m_numRRCConnRej++;
    }

    // -----------------------------------------
    inline void StsCounter::countAllocDlHarqFailure() {
    	m_allocDlHarqFailure++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countInvalidState() {
    	m_invalidState++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupTimeout() {
        m_numRRCSetupTimeout++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countBSRTimeout() {
        m_numBsrTimeout++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countTACmdRecvd() {
        m_numTACmdRecvd++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countForceULGrantRecvd() {
        m_numForceULGrantRecvd++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countNonConsecutiveSfnSf() {
        m_nonConsecutiveSfnSf++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countTestSuccess() {
        m_succTest++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countTestFailure() {
        m_failureTest++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRachSent() {
        m_rachSent++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRachTimeout() {
        m_rachTimeout++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRarRecvd() {
        m_rarRecvd++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRarInvalid() {
        m_rarInvalid++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countMsg3ULCfgRecvd() {
#if 0
        m_msg3UlCfgRecvd++;
#endif
    }

    // -----------------------------------------
    inline void StsCounter::countMsg3Sent() {
        m_msg3Sent++;
    }

    // -----------------------------------------
    inline void StsCounter::countMsg3CrcSent() {
#if 0
        m_msg3CrcSent++;
#endif
    }  

    // -----------------------------------------
    inline void StsCounter::countContentionResolutionRecvd() {
        m_contentionResolutionRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countContentionResolutionTimeout() {
        m_contentionResolutionTimeout++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countContentionResolutionInvalid() {
        m_contentionResolutionInvalid++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countHarqAckSent() {
        m_harqAckSent++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupRecvd(){
        m_rrcSetupRecvd++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupInvalid() {
        m_rrcSetupInvalid++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countSRSent() { 
        m_srSent++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countSRTimeout() {
        m_srTimeout++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplDCI0Recvd() {
#if 0
        m_rrcSetupComplDCI0Recvd++;
#endif
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplULCfgRecvd() {
#if 0
        m_rrcSetupComplUlCfgRecvd++;
#endif
    }

    
    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplSent() {
        m_rrcSetupComplSent++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplCrcSent() {
#if 0
        m_rrcSetupComplCrcSent++;
#endif
    }

    // -----------------------------------------
    inline void StsCounter::countIdentityRequestTimeout() {
        m_identityReqTimeout++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countHarqTimeout() {
        m_harqTimeout++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countHarqAckRecvd() {
        m_harqAckRecvd++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countHarqNAckRecvd() {
        m_harqNAckRecvd++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplHarqTimeout() {
        m_rrcSetupComplHarqTimeout++;
        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countIdentityRequestRecvd() {
        m_identityReqRecvd++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countIdentityResponseSent() {
        m_identityRespSent++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countAttachRejectRecvd() {
        m_attachRejectRecvd++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCRelRecvd() {
        m_rrcReleaseRecvd++;
//        m_isChanged = TRUE;
    }

    // -----------------------------------------
    inline void StsCounter::countT300Timeout() {
        m_t300Timeout++;
        m_isChanged = TRUE;
    }

}

#endif
