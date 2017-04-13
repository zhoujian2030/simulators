/*
 * UESuspending.cpp
 *
 *  Created on: 2017-3-16
 *      Author: j.zhou
 */

#include "UESuspending.h"
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
UESuspending::UESuspending(UInt8 ueId, UInt16 raRnti, PhyMacAPI* phyMacAPI, StsCounter* stsCounter)
: UeTerminal(ueId, raRnti, phyMacAPI, stsCounter)
{

}

// --------------------------------------------
UESuspending::~UESuspending() {

}

// --------------------------------------------
void UESuspending::ulHarqResultCallback(UInt16 harqId, BOOL result, UInt8 ueState) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, result = %d, ueState = %d, harqId = %d, \n",  __func__,
        m_uniqueId, result, ueState, harqId);

    if (result == TRUE) {
        if (ueState == RRC_SETUP_COMPLETE_SENT && m_state < RRC_CONNECTED) {
            this->stopT300();
#if 0
            m_state = RRC_CONNECTED;
#else
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, suspend the UE, wait RRC terminated\n",  __func__, m_uniqueId);
            m_suspend = TRUE;
            m_state = RRC_RELEASING;
#endif
        } else if (ueState == RRC_SETUP_COMPLETE_BSR_SENT) {
            m_state = RRC_SETUP_COMPLETE_BSR_ACK_RECVD;
        } else {
            if ((m_state >= RRC_SETUP_COMPLETE_SENT) && (m_state <= RRC_CONNECTED)) {
                m_subState = SUB_ST_IDLE;
            }
        }
    } else {
        if (ueState == RRC_SETUP_COMPLETE_SENT) {
            // receive nack for RRC setup complete,
            // terminate the connection later
            this->stopT300();
            m_state = WAIT_TERMINATING;
        } else {
            // TODO
        }
    }

    LOG_DBG(UE_LOGGER_NAME, "[%s], %s, change m_state to %d\n",  __func__, m_uniqueId, m_state);
}

// --------------------------------------------
void UESuspending::rrcCallback(UInt32 msgType, RrcMsg* msg) {
    LOG_TRACE(UE_LOGGER_NAME, "[%s], %s, msgType = %d\n",  __func__, m_uniqueId, msgType);

    switch (msgType) {
        case IDENTITY_REQUEST:
        {
            m_stsCounter->countIdentityRequestRecvd();
            m_triggerIdRsp = TRUE;
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Identity Request, will send Identity Resp later\n",  __func__, m_uniqueId);

            // TODO if receive identity request again ??

            requestUlResource();

            break;
        }

        case ATTACH_REJECT:
        {
            m_stsCounter->countAttachRejectRecvd();
            LOG_INFO(UE_LOGGER_NAME, "[%s], %s, recv Attach Reject\n",  __func__, m_uniqueId);
#if 0
			LOG_INFO(UE_LOGGER_NAME, "[%s], %s, suspend the UE, wait RRC terminated\n",  __func__, m_uniqueId);
			m_suspend = TRUE;
			m_state = RRC_RELEASING;
#endif
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
