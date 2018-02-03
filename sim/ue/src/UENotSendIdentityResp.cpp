/*
 * UENotSendIdentityResp.cpp
 *
 *  Created on: Jan 30, 2018
 *      Author: zhoujian
 */

#include "UENotSendIdentityResp.h"
#ifdef OS_LINUX
#include "CLogger.h"
#else
#include "logger.h"
#endif
#include "PhyMacAPI.h"
#include "UeScheduler.h"
#include "StsCounter.h"

using namespace ue;

// --------------------------------------------
UENotSendIdentityResp::UENotSendIdentityResp(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, preamble, phyMacAPI, stsCounter)
{
	// TODO Auto-generated constructor stub

}

// -------------------------------------------
UENotSendIdentityResp::~UENotSendIdentityResp() {
	// TODO Auto-generated destructor stub
}

// -------------------------------------------
void UENotSendIdentityResp::rrcCallback(UInt32 msgType, RrcMsg* msg) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, msgType = %d\n",  __func__, m_uniqueId, msgType);

    switch (msgType) {
        case IDENTITY_REQUEST:
        {
            m_stsCounter->countIdentityRequestRecvd();
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Identity Request, will  NOT send Identity Resp\n",  __func__, m_uniqueId);

            break;
        }

        case ATTACH_REJECT:
        {
            m_stsCounter->countAttachRejectRecvd();
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Attach Reject\n",  __func__, m_uniqueId);
            break;
        }

        case RRC_RELEASE:
        {
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

