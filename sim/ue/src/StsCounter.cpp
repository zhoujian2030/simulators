/*
 * StsCounter.cpp
 *
 *  Created on: Nov 08, 2016
 *      Author: j.zhou
 */

#include "StsCounter.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "../sysService/common/logger.h"
#endif

using namespace ue;

StsCounter* StsCounter::m_theInstance = 0;

// ------------------------------------------------
StsCounter::StsCounter() 
: m_succTest(0), m_failureTest(0), m_rachSent(0), m_rachTimeout(0), /*m_rachTimeoutMissSchPdu(0),
  m_rarDciRecvd(0), m_rarDciInvalid()0, */m_rarRecvd(0), m_rarInvalid(0),
  m_msg3UlCfgRecvd(0), m_msg3Sent(0), m_msg3CrcSent(0), m_contentionResolutionRecvd(0),
  m_contentionResolutionInvalid(0), m_contentionResolutionTimeout(0), m_harqAckSent(0),
  m_rrcSetupRecvd(0),m_rrcSetupInvalid(0), m_srSent(0), m_srTimeout(0), m_rrcSetupComplDCI0Recvd(0),
  m_rrcSetupComplUlCfgRecvd(0), m_rrcSetupComplSent(0), m_rrcSetupComplCrcSent(0), m_rrcSetupComplHarqTimeout(0),
  m_harqTimeout(0), m_harqAckRecvd(0), m_harqNAckRecvd(0), m_identityReqRecvd(0), m_identityReqTimeout(0),
  m_identityRespSent(0), m_attachRejectRecvd(0), m_rrcReleaseRecvd(0), m_t300Timeout(0), m_nonConsecutiveSfnSf(0),
  m_numTACmdRecvd(0), m_numForceULGrantRecvd(0), m_numBsrTimeout(0), m_numRRCSetupTimeout(0)
{

}

// ------------------------------------------------
StsCounter::~StsCounter() {
    
}

// ------------------------------------------------
StsCounter* StsCounter::getInstance() {
    if (m_theInstance == 0) {
        m_theInstance = new StsCounter();
        return m_theInstance;
    }

    return m_theInstance;
}

// ------------------------------------------------
void StsCounter::show() {
    LOG_INFO(UE_LOGGER_NAME, "==========================================\n");

    LOG_INFO(UE_LOGGER_NAME, " Num Success Test:                     %d\n", m_succTest);
    LOG_INFO(UE_LOGGER_NAME, " Num Failure Test:                     %d\n", m_failureTest);
    LOG_INFO(UE_LOGGER_NAME, " Num RACH sent:                        %d\n", m_rachSent);
    LOG_INFO(UE_LOGGER_NAME, " Num RAR received:                     %d\n", m_rarRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num MSG3 UL CFG received:             %d\n", m_msg3UlCfgRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num MSG3 sent:                        %d\n", m_msg3Sent);
    LOG_INFO(UE_LOGGER_NAME, " Num MSG3 CRC sent:                    %d\n", m_msg3CrcSent);
    LOG_INFO(UE_LOGGER_NAME, " Num Contention Resolution received:   %d\n", m_contentionResolutionRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup received:               %d\n", m_rrcSetupRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num HARQ ACK sent:                    %d\n", m_harqAckSent);
    LOG_INFO(UE_LOGGER_NAME, " Num SR sent:                          %d\n", m_srSent);
    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete DCI0 recvd:    %d\n", m_rrcSetupComplDCI0Recvd);
    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete UL CFG recvd:  %d\n", m_rrcSetupComplUlCfgRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete sent:          %d\n", m_rrcSetupComplSent);
    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete CRC sent:      %d\n", m_rrcSetupComplCrcSent);
    LOG_INFO(UE_LOGGER_NAME, " Num HARQ ACK received:                %d\n", m_harqAckRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num Identity Request recvd:           %d\n", m_identityReqRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num Identity Response sent:           %d\n", m_identityRespSent);
    LOG_INFO(UE_LOGGER_NAME, " Num Attach Reject recvd:              %d\n", m_attachRejectRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num RRC Release recvd:                %d\n", m_rrcReleaseRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num TA Command recvd:                 %d\n", m_numTACmdRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num Force UL Grant recvd:             %d\n", m_numForceULGrantRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num T300 timeout:                     %d\n", m_t300Timeout);
    LOG_INFO(UE_LOGGER_NAME, " Num RACH timeout:                     %d\n", m_rachTimeout);
    LOG_INFO(UE_LOGGER_NAME, " Num RAR invalid:                      %d\n", m_rarInvalid);
    LOG_INFO(UE_LOGGER_NAME, " Num Contention Resolution invalid:    %d\n", m_contentionResolutionInvalid);
    LOG_INFO(UE_LOGGER_NAME, " Num Contention Resolution timeout:    %d\n", m_contentionResolutionTimeout);
    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup timeout:                %d\n", m_numRRCSetupTimeout);    
    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup invalid:                %d\n", m_rrcSetupInvalid);
    LOG_INFO(UE_LOGGER_NAME, " Num SR timeout:                       %d\n", m_srTimeout);
    LOG_INFO(UE_LOGGER_NAME, " Num BSR timeout:                      %d\n", m_numBsrTimeout);
    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete HARQ timeout:  %d\n", m_rrcSetupComplHarqTimeout);
    LOG_INFO(UE_LOGGER_NAME, " Num HARQ timeout:                     %d\n", m_harqTimeout);    
    LOG_INFO(UE_LOGGER_NAME, " Num HARQ NACK received:               %d\n", m_harqNAckRecvd);
    LOG_INFO(UE_LOGGER_NAME, " Num Identity Request timeout:         %d\n", m_identityReqTimeout);    
    LOG_INFO(UE_LOGGER_NAME, " Num Invalid Msg with invalid sfnsf:   %d\n", m_nonConsecutiveSfnSf);

    LOG_INFO(UE_LOGGER_NAME, "===========================================\n"); 
}