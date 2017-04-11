/*
 * StsCounter.cpp
 *
 *  Created on: Nov 08, 2016
 *      Author: j.zhou
 */

#include "StsCounter.h"
#include <stdio.h>
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "../sysService/common/logger.h"
#endif

using namespace ue;

StsCounter* StsCounter::m_theInstance = 0;

// ------------------------------------------------
StsCounter::StsCounter() 
: m_isChanged(TRUE), m_succTest(0), m_failureTest(0), m_rachSent(0), m_rachTimeout(0), /*m_rachTimeoutMissSchPdu(0),
  m_rarDciRecvd(0), m_rarDciInvalid()0, */m_rarRecvd(0), m_rarInvalid(0),
  m_msg3UlCfgRecvd(0), m_msg3Sent(0), m_msg3CrcSent(0), m_contentionResolutionRecvd(0),
  m_contentionResolutionInvalid(0), m_contentionResolutionTimeout(0), m_harqAckSent(0),
  m_rrcSetupRecvd(0),m_rrcSetupInvalid(0), m_srSent(0), m_srTimeout(0), m_rrcSetupComplDCI0Recvd(0),
  m_rrcSetupComplUlCfgRecvd(0), m_rrcSetupComplSent(0), m_rrcSetupComplCrcSent(0), m_rrcSetupComplHarqTimeout(0),
  m_harqTimeout(0), m_harqAckRecvd(0), m_harqNAckRecvd(0), m_identityReqRecvd(0), m_identityReqTimeout(0),
  m_identityRespSent(0), m_attachRejectRecvd(0), m_rrcReleaseRecvd(0), m_t300Timeout(0), m_nonConsecutiveSfnSf(0),
  m_numTACmdRecvd(0), m_numForceULGrantRecvd(0), m_numBsrTimeout(0), m_numRRCSetupTimeout(0), m_invalidState(0),
  m_allocDlHarqFailure(0), m_numRRCReestabRej(0), m_numRRCConnRej(0), m_numSuccRejTest(0), m_rrcSetupRetransmit(0),
  m_identityReqRetransmit(0)
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
	if (!m_isChanged) {
		return;
	}
	m_isChanged = FALSE;

	Char8 stsData[2048];
	UInt32 sumLength = 0;
	SInt32 varDataLen = 0;

	varDataLen = sprintf(stsData + sumLength, "[%s]\n==========================================\n", __func__);
	sumLength += varDataLen;
	varDataLen = sprintf(stsData + sumLength, " Num Success Test:        %d\n", m_succTest);
	sumLength += varDataLen;
	varDataLen = sprintf(stsData + sumLength, " Num Success Reject Test: %d\n", m_numSuccRejTest);
	sumLength += varDataLen;
	varDataLen = sprintf(stsData + sumLength, " Num Failure Test:        %d\n", m_failureTest);
	sumLength += varDataLen;
	if (m_rachSent > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RACH sent:           %d\n", m_rachSent);
		sumLength += varDataLen;
	}
	if (m_rarRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RAR received:        %d\n", m_rarRecvd);
		sumLength += varDataLen;
	}
	if (m_msg3UlCfgRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num MSG3 ULCFG received: %d\n", m_msg3UlCfgRecvd);
		sumLength += varDataLen;
	}
	if (m_msg3Sent > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num MSG3 sent:           %d\n", m_msg3Sent);
		sumLength += varDataLen;
	}
	if (m_msg3CrcSent > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num MSG3 CRC sent:       %d\n", m_msg3CrcSent);
		sumLength += varDataLen;
	}
	if (m_contentionResolutionRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Cont Resol received: %d\n", m_contentionResolutionRecvd);
		sumLength += varDataLen;
	}
	if (m_rrcSetupRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRC Setup received:  %d\n", m_rrcSetupRecvd);
		sumLength += varDataLen;
	}
	if (m_rrcSetupRetransmit > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRC Setup retrans:   %d\n", m_rrcSetupRetransmit);
		sumLength += varDataLen;
	}
	if (m_numRRCReestabRej > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRC Restablish Rej:  %d\n", m_numRRCReestabRej);
		sumLength += varDataLen;
	}
	if (m_numRRCConnRej > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRC Connection Rej:  %d\n", m_numRRCConnRej);
		sumLength += varDataLen;
	}
	if (m_harqAckSent > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num HARQ ACK sent:       %d\n", m_harqAckSent);
		sumLength += varDataLen;
	}
	if (m_srSent > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num SR sent:             %d\n", m_srSent);
		sumLength += varDataLen;
	}
	if (m_rrcSetupComplSent > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRCSetup compl sent: %d\n", m_rrcSetupComplSent);
		sumLength += varDataLen;
	}
	if (m_rrcSetupComplCrcSent > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRCSetup compl CRC : %d\n", m_rrcSetupComplCrcSent);
		sumLength += varDataLen;
	}
	if (m_harqAckRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num HARQ ACK received:   %d\n", m_harqAckRecvd);
		sumLength += varDataLen;
	}
	if (m_identityReqRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Identity Req recvd:   %d\n", m_identityReqRecvd);
		sumLength += varDataLen;
	}
	if (m_identityReqRetransmit > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Identity Req retrans: %d\n", m_identityReqRetransmit);
		sumLength += varDataLen;
	}
	if (m_identityRespSent > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Identity Resp sent:  %d\n", m_identityRespSent);
		sumLength += varDataLen;
	}
	if (m_attachRejectRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Attach Reject recvd: %d\n", m_attachRejectRecvd);
		sumLength += varDataLen;
	}
	if (m_rrcReleaseRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRC Release recvd:   %d\n", m_rrcReleaseRecvd);
		sumLength += varDataLen;
	}
	if (m_numTACmdRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num TA Command recvd:    %d\n", m_numTACmdRecvd);
		sumLength += varDataLen;
	}
	if (m_numForceULGrantRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Force ULGrant recvd: %d\n", m_numForceULGrantRecvd);
		sumLength += varDataLen;
	}
	if (m_invalidState > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Invalid State:       %d\n", m_invalidState);
		sumLength += varDataLen;
	}
	if (m_allocDlHarqFailure > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Alloc DL HARQ fail:  %d\n", m_allocDlHarqFailure);
		sumLength += varDataLen;
	}
	if (m_t300Timeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num T300 timeout:        %d\n", m_t300Timeout);
		sumLength += varDataLen;
	}
	if (m_rachTimeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RACH timeout:        %d\n", m_rachTimeout);
		sumLength += varDataLen;
	}
	if (m_rarInvalid > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RAR invalid:         %d\n", m_rarInvalid);
		sumLength += varDataLen;
	}
	if (m_numRRCSetupTimeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRC setup timeout:   %d\n", m_numRRCSetupTimeout);
		sumLength += varDataLen;
	}
	if (m_rrcSetupInvalid > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRC setup invalid:   %d\n", m_rrcSetupInvalid);
		sumLength += varDataLen;
	}
	if (m_srTimeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num SR timeout:          %d\n", m_srTimeout);
		sumLength += varDataLen;
	}
	if (m_numBsrTimeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num BSR timeout:         %d\n", m_numBsrTimeout);
		sumLength += varDataLen;
	}
	if (m_harqTimeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num HARQ timeout:        %d\n", m_harqTimeout);
		sumLength += varDataLen;
	}
	if (m_harqNAckRecvd > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num HARQ NACK received:  %d\n", m_harqNAckRecvd);
		sumLength += varDataLen;
	}
	if (m_contentionResolutionInvalid > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Cont Resol invalid:  %d\n", m_contentionResolutionInvalid);
		sumLength += varDataLen;
	}
	if (m_contentionResolutionTimeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Cont Resol timeout:  %d\n", m_contentionResolutionTimeout);
		sumLength += varDataLen;
	}
	if (m_identityReqTimeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Identity Req timeout:           %d\n", m_identityReqTimeout);
		sumLength += varDataLen;
	}
	if (m_rrcSetupComplHarqTimeout > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num RRCSetup compl HARQ timeout:    %d\n", m_rrcSetupComplHarqTimeout);
		sumLength += varDataLen;
	}
	if (m_nonConsecutiveSfnSf > 0) {
		varDataLen = sprintf(stsData + sumLength, " Num Invalid Msg with invalid sfnsf: %d\n", m_nonConsecutiveSfnSf);
		sumLength += varDataLen;
	}
	varDataLen = sprintf(stsData + sumLength, "==========================================\n");
	sumLength += varDataLen;
	stsData[sumLength + 1] = '\0';

    LOG_INFO(UE_LOGGER_NAME, "%s", stsData);

//    LOG_INFO(UE_LOGGER_NAME, " Num Success Test:                     %ld\n", m_succTest);
//    LOG_INFO(UE_LOGGER_NAME, " Num Failure Test:                     %ld\n", m_failureTest);
//    LOG_INFO(UE_LOGGER_NAME, " Num RACH sent:                        %ld\n", m_rachSent);
//    LOG_INFO(UE_LOGGER_NAME, " Num RAR received:                     %ld\n", m_rarRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num MSG3 UL CFG received:             %ld\n", m_msg3UlCfgRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num MSG3 sent:                        %ld\n", m_msg3Sent);
//    LOG_INFO(UE_LOGGER_NAME, " Num MSG3 CRC sent:                    %ld\n", m_msg3CrcSent);
//    LOG_INFO(UE_LOGGER_NAME, " Num Contention Resolution received:   %ld\n", m_contentionResolutionRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup received:               %ld\n", m_rrcSetupRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num HARQ ACK sent:                    %ld\n", m_harqAckSent);
//    LOG_INFO(UE_LOGGER_NAME, " Num SR sent:                          %ld\n", m_srSent);
////    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete DCI0 recvd:    %ld\n", m_rrcSetupComplDCI0Recvd);
////    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete UL CFG recvd:  %ld\n", m_rrcSetupComplUlCfgRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete sent:          %ld\n", m_rrcSetupComplSent);
//    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete CRC sent:      %ld\n", m_rrcSetupComplCrcSent);
//    LOG_INFO(UE_LOGGER_NAME, " Num HARQ ACK received:                %ld\n", m_harqAckRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num Identity Request recvd:           %ld\n", m_identityReqRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num Identity Response sent:           %ld\n", m_identityRespSent);
//    LOG_INFO(UE_LOGGER_NAME, " Num Attach Reject recvd:              %ld\n", m_attachRejectRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num RRC Release recvd:                %ld\n", m_rrcReleaseRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num TA Command recvd:                 %ld\n", m_numTACmdRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num Force UL Grant recvd:             %ld\n", m_numForceULGrantRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num T300 timeout:                     %ld\n", m_t300Timeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num RACH timeout:                     %ld\n", m_rachTimeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num RAR invalid:                      %ld\n", m_rarInvalid);
//    LOG_INFO(UE_LOGGER_NAME, " Num Contention Resolution invalid:    %ld\n", m_contentionResolutionInvalid);
//    LOG_INFO(UE_LOGGER_NAME, " Num Contention Resolution timeout:    %ld\n", m_contentionResolutionTimeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup timeout:                %ld\n", m_numRRCSetupTimeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup invalid:                %ld\n", m_rrcSetupInvalid);
//    LOG_INFO(UE_LOGGER_NAME, " Num SR timeout:                       %ld\n", m_srTimeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num BSR timeout:                      %ld\n", m_numBsrTimeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num RRC setup complete HARQ timeout:  %ld\n", m_rrcSetupComplHarqTimeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num HARQ timeout:                     %ld\n", m_harqTimeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num HARQ NACK received:               %ld\n", m_harqNAckRecvd);
//    LOG_INFO(UE_LOGGER_NAME, " Num Identity Request timeout:         %ld\n", m_identityReqTimeout);
//    LOG_INFO(UE_LOGGER_NAME, " Num Invalid Msg with invalid sfnsf:   %ld\n", m_nonConsecutiveSfnSf);
//
//    LOG_INFO(UE_LOGGER_NAME, "===========================================\n");
}