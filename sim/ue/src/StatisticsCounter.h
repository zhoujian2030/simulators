/*
 * StatisticsCounter.h
 *
 *  Created on: Nov 08, 2016
 *      Author: j.zhou
 */

#ifndef STATISTICS_COUNTER_H
#define STATISTICS_COUNTER_H

#include "UeBaseType.h"

namespace ue {

    class StatisticsCounter {
    public:
        ~StatisticsCounter();

        static StatisticsCounter* getInstance();

        void show();

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

    private:
        StatisticsCounter();

        static StatisticsCounter* m_theInstance;

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
    };

    // -----------------------------------------
    inline void StatisticsCounter::countRachSent() {
        m_rachSent++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRachTimeout() {
        m_rachTimeout++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRarRecvd() {
        m_rarRecvd++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRarInvalid() {
        m_rarInvalid++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countMsg3ULCfgRecvd() {
        m_msg3UlCfgRecvd++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countMsg3Sent() {
        m_msg3Sent++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countMsg3CrcSent() {
        m_msg3CrcSent++;
    }  

    // -----------------------------------------
    inline void StatisticsCounter::countContentionResolutionRecvd() {
        m_contentionResolutionRecvd++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countContentionResolutionTimeout() {
        m_contentionResolutionTimeout++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countContentionResolutionInvalid() {
        m_contentionResolutionInvalid++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countHarqAckSent() {
        m_harqAckSent++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRRCSetupRecvd(){
        m_rrcSetupRecvd++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRRCSetupInvalid() {
        m_rrcSetupInvalid++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countSRSent() { 
        m_srSent++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countSRTimeout() {
        m_srTimeout++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRRCSetupComplDCI0Recvd() {
        m_rrcSetupComplDCI0Recvd++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRRCSetupComplULCfgRecvd() {
        m_rrcSetupComplUlCfgRecvd++;
    }

    
    // -----------------------------------------
    inline void StatisticsCounter::countRRCSetupComplSent() {
        m_rrcSetupComplSent++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRRCSetupComplCrcSent() {
        m_rrcSetupComplCrcSent++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countIdentityRequestTimeout() {
        m_identityReqTimeout++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countHarqTimeout() {
        m_harqTimeout++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countHarqAckRecvd() {
        m_harqAckRecvd++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countHarqNAckRecvd() {
        m_harqNAckRecvd++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countRRCSetupComplHarqTimeout() {
        m_rrcSetupComplHarqTimeout++;
    }

    // -----------------------------------------
    inline void StatisticsCounter::countIdentityRequestRecvd() {
        m_identityReqRecvd++;
    }
}

#endif
