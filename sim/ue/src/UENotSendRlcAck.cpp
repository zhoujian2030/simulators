/*
 * UENotSendRlcAck.cpp
 *
 *  Created on: 2017-3-16
 *      Author: j.zhou
 */

#include "UENotSendRlcAck.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "../sysService/common/logger.h"
#endif
#include "PhyMacAPI.h"
#include "UeScheduler.h"
#include "StsCounter.h"

using namespace ue;

// --------------------------------------------
UENotSendRlcAck::UENotSendRlcAck(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, phyMacAPI, stsCounter)
{
	m_numIdentityReqRecvd = 0;
	m_numAttachRejRecvd = 0;
	m_numRRCRelRecvd = 0;
	m_maxRachIntervalSfn = 0;
}

// --------------------------------------------
UENotSendRlcAck::~UENotSendRlcAck() {

}

// --------------------------------------------
void UENotSendRlcAck::resetChild() {
	m_numIdentityReqRecvd = 0;
	m_numAttachRejRecvd = 0;
	m_numRRCRelRecvd = 0;
}

// --------------------------------------------
void UENotSendRlcAck::rrcCallback(UInt32 msgType, RrcMsg* msg) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, msgType = %d\n",  __func__, m_uniqueId, msgType);

    switch (msgType) {
        case IDENTITY_REQUEST:
        {
        	m_numIdentityReqRecvd++;
        	m_stsCounter->countIdentityRequestRecvd();
//        	if (m_numIdentityReqRecvd == 2) {
				m_triggerIdRsp = TRUE;
				LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Identity Request, will send Identity Resp later\n",  __func__, m_uniqueId);

				// TODO if receive identity request again ??

				requestUlResource();
//        	} else {
//        		LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Identity Request, DO NOT send Identity Resp\n",  __func__, m_uniqueId);
//        	}

            break;
        }

        case ATTACH_REJECT:
        {
        	m_numAttachRejRecvd++;
            m_stsCounter->countAttachRejectRecvd();
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Attach Reject\n",  __func__, m_uniqueId);
            break;
        }

        case RRC_RELEASE:
        {
        	m_numRRCRelRecvd++;
            m_stsCounter->countRRCRelRecvd();
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv RRC Release, will free resource later\n",  __func__, m_uniqueId);
            LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change state from %d to %d\n",  __func__, m_uniqueId, m_state, RRC_RELEASING);
            m_state = RRC_RELEASING;
            break;
        }

        default:
        {
            LOG_ERROR(UE_LOGGER_NAME, "[%s], %s, Unsupported msgType = %d\n",  __func__, m_uniqueId, msgType);
        }
    }
}

// --------------------------------------------
void UENotSendRlcAck::rlcCallback(UInt8* statusPdu, UInt32 length) {
	if (m_state >= RRC_CONNECTED) {
		if (m_numRRCRelRecvd > 0 || m_numAttachRejRecvd > 0) {
			LOG_INFO(UE_LOGGER_NAME, "[%s], %s, DO NOT send RLC ACK for rrc release and attach reject, wait network RLC retransmit\n",  __func__, m_uniqueId);
			return;
		}
//		if (m_numIdentityReqRecvd != 0) {
//			if (m_numIdentityReqRecvd == 1) {
//				LOG_INFO(UE_LOGGER_NAME, "[%s], %s, DO NOT send RLC ACK for identity request, wait network RLC retransmit\n",  __func__, m_uniqueId);
//				return;
//			} else {
////				m_numIdentityReqRecvd = 0;
//				if (m_numAttachRejRecvd != 0) {
//					if (m_numAttachRejRecvd == 1) {
//						LOG_INFO(UE_LOGGER_NAME, "[%s], %s, DO NOT send RLC ACK for attach reject, wait network RLC retransmit\n",  __func__, m_uniqueId);
//						m_numAttachRejRecvd = 2;
//						// why attach reject not retransmit???
//						return;
//					} else {
////						m_numAttachRejRecvd = 0;
//						if (m_numRRCRelRecvd != 0) {
//							if (m_numRRCRelRecvd == 1) {
//								LOG_INFO(UE_LOGGER_NAME, "[%s], %s, DO NOT send RLC ACK for rrc release, wait network RLC retransmit\n",  __func__, m_uniqueId);
//								return;
//							} else {
//				//				m_numRRCRelRecvd = 0;
//								LOG_INFO(UE_LOGGER_NAME, "[%s], %s, DO NOT send RLC ACK for rrc release, wait network RLC retransmit\n",  __func__, m_uniqueId);
//								return;
//							}
//						}
//					}
//				} else {
//					// TODO
//				}
//			}
//		}

	}

	LOG_INFO(UE_LOGGER_NAME, "[%s], %s, need to send RLC status PDU, length = %d\n",  __func__, m_uniqueId, length);

	if (length != 2) {
		LOG_WARN(UE_LOGGER_NAME, "[%s], %s, Only support 2 bytes format RLC Status PDU now\n",  __func__, m_uniqueId);
		return;
	}

	m_triggerRlcStatusPdu = TRUE;
	requestUlResource();
	memcpy(&m_rlcStatusPdu, statusPdu, length);
}


