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

    private:
        StsCounter();

        static StsCounter* m_theInstance;

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
    };

    // -----------------------------------------
    inline void StsCounter::countRRCSetupTimeout() {
        m_numRRCSetupTimeout++;
    }

    // -----------------------------------------
    inline void StsCounter::countBSRTimeout() {
        m_numBsrTimeout++;
    }

    // -----------------------------------------
    inline void StsCounter::countTACmdRecvd() {
        m_numTACmdRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countForceULGrantRecvd() {
        m_numForceULGrantRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countNonConsecutiveSfnSf() {
        m_nonConsecutiveSfnSf++;
    }

    // -----------------------------------------
    inline void StsCounter::countTestSuccess() {
        m_succTest++;
    }

    // -----------------------------------------
    inline void StsCounter::countTestFailure() {
        m_failureTest++;
    }

    // -----------------------------------------
    inline void StsCounter::countRachSent() {
        m_rachSent++;
    }

    // -----------------------------------------
    inline void StsCounter::countRachTimeout() {
        m_rachTimeout++;
    }

    // -----------------------------------------
    inline void StsCounter::countRarRecvd() {
        m_rarRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countRarInvalid() {
        m_rarInvalid++;
    }

    // -----------------------------------------
    inline void StsCounter::countMsg3ULCfgRecvd() {
        m_msg3UlCfgRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countMsg3Sent() {
        m_msg3Sent++;
    }

    // -----------------------------------------
    inline void StsCounter::countMsg3CrcSent() {
        m_msg3CrcSent++;
    }  

    // -----------------------------------------
    inline void StsCounter::countContentionResolutionRecvd() {
        m_contentionResolutionRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countContentionResolutionTimeout() {
        m_contentionResolutionTimeout++;
    }

    // -----------------------------------------
    inline void StsCounter::countContentionResolutionInvalid() {
        m_contentionResolutionInvalid++;
    }

    // -----------------------------------------
    inline void StsCounter::countHarqAckSent() {
        m_harqAckSent++;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupRecvd(){
        m_rrcSetupRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupInvalid() {
        m_rrcSetupInvalid++;
    }

    // -----------------------------------------
    inline void StsCounter::countSRSent() { 
        m_srSent++;
    }

    // -----------------------------------------
    inline void StsCounter::countSRTimeout() {
        m_srTimeout++;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplDCI0Recvd() {
        m_rrcSetupComplDCI0Recvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplULCfgRecvd() {
        m_rrcSetupComplUlCfgRecvd++;
    }

    
    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplSent() {
        m_rrcSetupComplSent++;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplCrcSent() {
        m_rrcSetupComplCrcSent++;
    }

    // -----------------------------------------
    inline void StsCounter::countIdentityRequestTimeout() {
        m_identityReqTimeout++;
    }

    // -----------------------------------------
    inline void StsCounter::countHarqTimeout() {
        m_harqTimeout++;
    }

    // -----------------------------------------
    inline void StsCounter::countHarqAckRecvd() {
        m_harqAckRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countHarqNAckRecvd() {
        m_harqNAckRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCSetupComplHarqTimeout() {
        m_rrcSetupComplHarqTimeout++;
    }

    // -----------------------------------------
    inline void StsCounter::countIdentityRequestRecvd() {
        m_identityReqRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countIdentityResponseSent() {
        m_identityRespSent++;
    }

    // -----------------------------------------
    inline void StsCounter::countAttachRejectRecvd() {
        m_attachRejectRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countRRCRelRecvd() {
        m_rrcReleaseRecvd++;
    }

    // -----------------------------------------
    inline void StsCounter::countT300Timeout() {
        m_t300Timeout++;
    }

}

#endif
