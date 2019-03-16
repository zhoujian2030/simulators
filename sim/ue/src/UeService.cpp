/*
 * UeService.h
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifdef OS_LINUX

#include "UeService.h"
#include <iostream>
#include "CLogger.h"

using namespace ue;
using namespace net;

// ------------------------------------------------
UeService::UeService(std::string serviceName) 
: Service(serviceName)
{
    init();
}

// ------------------------------------------------
UeService::~UeService() {

}

// ------------------------------------------------
unsigned long UeService::run() {

    // SfnSfManager* pSfnSfMgr = SfnSfManager::getInstance();
    // UInt32 sfn, sf;

    // Socket::InetAddressPort remoteAddress;
    // Int32 recvLength;   

    m_phyMacAPI = new PhyMacAPI(m_recvBuffer);

    m_udpServerSocket = new UdpSocket("0.0.0.0", 9999);
    m_udpServerSocket->addSocketHandlerForNonAsync(m_phyMacAPI, (char*)m_recvBuffer, SOCKET_BUFFER_LENGTH);

    m_selectSocketSet = new SelectSocketSet();
    m_selectSocketSet->registerInputHandler(m_udpServerSocket, (SocketEventHandler*)m_udpServerSocket);    

    SelectSocketSet::SelectSocket* readySockets;
    
    while(1) {    
        readySockets = ( SelectSocketSet::SelectSocket*)m_selectSocketSet->poll(-1);
        // only the first ready socket is valid
        readySockets->eventHandler->handleInput(readySockets->socket);

        // m_msEvent.wait();
        // pSfnSfMgr->getSFAndSFN(&sfn, &sf);
        // LOG_DBG(UE_LOGGER_NAME, "[%s], sfn = %d, sf = %d\n",  __func__, sfn, sf);
        // while(1) {
        //     recvLength = m_udpServerSocket->receive((Int8*)m_recvBuffer, SOCKET_BUFFER_LENGTH, remoteAddress);
        //     if ( recvLength <= 0) {
        //         //LOG_DBG(UE_LOGGER_NAME, "no data receive\n");
        //         break;
        //     }
        //     LOG_DBG(UE_LOGGER_NAME, "[%s], recv data (%d): \n",  __func__, recvLength);
        //     int n = 0;
        //     for (int i=0; i<recvLength; i++) {
        //         printf("%02x ", (UInt8)m_recvBuffer[i]);
        //         if (++n == 10) {
        //             n = 0;
        //             printf("\n");
        //         }
        //     }
        //     printf("\n");
        // }
    }

    return 0;
}

#else

extern "C" {

#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "sfnMgr.h"
#include "socketAdapter.h"
#include "task.h"
#include "msgQueue.h"
#include "pktQueue.h"
#include "platform.h"
#include "osCommon.h"
#include "msgMemMgr.h"
#include "logMgr.h"
#include "userTimer.h"
#include "lteTypes.h"
#include "UeService.h"

//#define CCS_SIM
extern void HandleSubFrameInd(void* phyUeSim, UInt16 sfnsf);
extern void HandleDlDataRequest(void* phyUeSim, UInt8* buffer, SInt32 length);
extern void UpdateUeConfig(void* phyUeSim, UInt32 numUe, UInt32 numAccessCount);
extern void* InitUePhySim();

#ifdef CCS_SIM
void MainEntryFun() {
	//UInt8 msgBuf[3000] = {0};
	//Int bytesRead = 0;
	void* pPhyUeSim = InitUePhySim();
	UInt16 sfn = 1;
	UInt8 sf = 1;
	UInt16 sfnsf = (sfn << 4) | sf;
	HandleSubFrameInd(pPhyUeSim, sfnsf);
}
#endif

/*
 *
 */
SInt32 RecvMsgFromQmss(void* msgBuf_p, Int32 qmssId)
{
    SInt32 bytesRead = 0;
	UInt8* pTempBuffer = PNULL;
	void* pTempQmssFd = PNULL;
	UInt32 count = 0;

	if((bytesRead = RecvMsgFromQmssQ(qmssId, Qmss_Type_RxHand, &pTempBuffer, &pTempQmssFd)) <= 0)
	{
		LOG_ERROR(MODULE_ID_OS_ADAPTER,
					   "phySim: [%s], RecvMsgFromQmssQ bytesRead[%d], rxQmssId[%d] failure.\n",
					   __func__, bytesRead, qmssId);
		count = 0;
		return count;
	}

	memcpy(msgBuf_p, pTempBuffer, bytesRead);

	callbackQmssFd(pTempQmssFd);

    return bytesRead;
}

extern S_SysSfn  gSysSfn;
volatile BOOL gSfnSfUpdated = FALSE;
BOOL gBroadcastRecvd = FALSE;
BOOL gCellConfigRecvd = FALSE;
BOOL gStartTest = FALSE;
void UePhySimTask(void)
{
	LOG_DBG(MODULE_ID_LAYER_MGR, "[%s], Entry\n", __func__);
	UInt32 phySimHandlerEventInfo = 0;
	UInt8 msgBuf[3000] = {0};
	Int bytesRead = 0;
	void* pPhyUeSim = InitUePhySim();

	while(1)
	{
		if(0 >= (UInt32)RecvMsg(&PhySimHandlerMsgQ_g, (void*)&phySimHandlerEventInfo, sizeof(UInt32), WAIT_FOREVER))
		{
			LOG_ERROR(MODULE_ID_LAYER_MGR,
					"phySim: [%s] RecvMsg phySimHandlerEventInfo[0x%08x] failure.\n",
					__func__, phySimHandlerEventInfo);
		}
//		LOG_INFO(MODULE_ID_LAYER_MGR, "[%s], running\n", __func__);

		UInt16 sfnsf = (gSysSfn.sfn << 4) | gSysSfn.sf;
		bytesRead = 0;

		if (gCellConfigRecvd) {

			UInt32 count = 0;
			UInt8 n = 0;

			// Receive DL Config request
			count = GetCountOfQmssQ(QMSS_RX_HAND_LAYER2C_FROM_L2_BUF_REP, Qmss_Type_RxHand);

			if (count > 0) {
				while ((n < count) && (n < 10)) {
					if( 0 != (bytesRead = RecvMsgFromQmss((void *)msgBuf, QMSS_RX_HAND_LAYER2C_FROM_L2_BUF_REP)))
					{
						if (gBroadcastRecvd && gStartTest) {
							LOG_DBG(MODULE_ID_LAYER_MGR, "[%s], recv DL Config request\n", __func__);
							HandleDlDataRequest(pPhyUeSim, msgBuf, bytesRead);
						}
					}

					n++;
				}
			}

			// Receive DL Data request
			count = GetCountOfQmssQ(QMSS_RX_HAND_LAYER2D_FROM_CMAC_SCH_RESULT, Qmss_Type_RxHand);
			n = 0;

			if (count > 0) {
				while ((n < count) && (n < 10)) {
					if( 0 != (bytesRead = RecvMsgFromQmss((void *)msgBuf, QMSS_RX_HAND_LAYER2D_FROM_CMAC_SCH_RESULT)))
					{
						if (!gBroadcastRecvd) {
							gBroadcastRecvd = TRUE;
						} else {
							if (gStartTest) {
								LOG_DBG(MODULE_ID_LAYER_MGR, "[%s], recv DL Data request\n", __func__);
								HandleDlDataRequest(pPhyUeSim, msgBuf, bytesRead);
							}
						}
					}

					n++;
				}
			}

			if (gStartTest) {
				// Receive UL Config / UL DCI / HI request
				count = GetCountOfQmssQ(QMSS_RX_HAND_LAYER2D_FROM_CMAC_CMAC_HARQ_ACK, Qmss_Type_RxHand);
				n = 0;
				if (count > 0) {
					while ((n < count) && (n < 10)) {
						if( 0 != (bytesRead = RecvMsgFromQmss((void *)msgBuf, QMSS_RX_HAND_LAYER2D_FROM_CMAC_CMAC_HARQ_ACK)))
						{
							if (gBroadcastRecvd) {
								LOG_DBG(MODULE_ID_LAYER_MGR, "[%s], recv UL Config / HI/DCI0 / UE Config Request\n", __func__);
								HandleDlDataRequest(pPhyUeSim, msgBuf, bytesRead);
							}
						}

						n++;
					}
				}
			}
		}

		if (!gCellConfigRecvd) {
			// Receive UE Config Request / Cell Config Request / start phy request
			if (GetCountOfQmssQ(QMSS_RX_HAND_LAYER2D_FROM_CMAC_CMAC_HARQ_ACK, Qmss_Type_RxHand) > 0) {
				if( 6 == (bytesRead = RecvMsgFromQmss((void *)msgBuf, QMSS_RX_HAND_LAYER2D_FROM_CMAC_CMAC_HARQ_ACK)))
				{
					LOG_INFO(MODULE_ID_LAYER_MGR, "[%s], recv PHY_START_REQUEST from qmss QMSS_RX_HAND_LAYER2D_FROM_CMAC_CMAC_HARQ_ACK\n", __func__);
					gCellConfigRecvd = TRUE;
				}
			}
		} else {
			if (!gStartTest) {
				if (GetCountOfQmssQ(QMSS_RX_HAND_LAYER2D_FROM_CMAC_CMAC_HARQ_ACK, Qmss_Type_RxHand) > 0) {
					if((bytesRead = RecvMsgFromQmss((void *)msgBuf, QMSS_RX_HAND_LAYER2D_FROM_CMAC_CMAC_HARQ_ACK)) > 0)
					{
						handleCliCommand(pPhyUeSim, msgBuf, bytesRead);
					}
				}
			}
		}

		if (gBroadcastRecvd && gStartTest) {
			HandleSubFrameInd(pPhyUeSim, sfnsf);
		}

	}
}



void handleCliCommand(void* pPhyUeSim, UInt8* pBuff, UInt32 length) {
	if (length < (CLI_MSG_HEAD_LENGTH + sizeof(SetSIMParamReq))) {
		LOG_ERROR(MODULE_ID_LAYER_MGR, "[%s], Invalid CLI msg, length = %d\n", __func__, length);
	}
    UInt16 msgId = 0;
    UInt16 srcModuleId   = 0;
    UInt16 destModuleId  = 0;
    UInt16 msgLen        = 0;
//    UInt16 transactionId = 0;
    UInt8* msg_p = pBuff;

//    transactionId =  LTE_GET_U16BIT(msg_p);
    msg_p += 2;
    srcModuleId   = LTE_GET_U16BIT(msg_p);
    msg_p += 2;
    destModuleId  = LTE_GET_U16BIT(msg_p);
    msg_p += 2;
    msgId         = LTE_GET_U16BIT(msg_p);
    msg_p += 2;
    msgLen        = LTE_GET_U16BIT(msg_p);
    msg_p += 2;

//	LOG_INFO(MODULE_ID_LAYER_MGR, "[%s], recv msg length = %d, srcModuleId = 0x%x, dstModuleId = 0x%x, msgId = %d, msgLen = %d\n", __func__,
//			length, srcModuleId, destModuleId, msgId, msgLen);

	if (msgId == SIM_CLI_SET_PARAM_REQ) {
		SetSIMParamReq* paramReq = (SetSIMParamReq*)(pBuff + CLI_MSG_HEAD_LENGTH);
		LOG_INFO(MODULE_ID_LAYER_MGR, "[%s], SET UE Number: %d, Test Time: %d\n", __func__, paramReq->numUe, paramReq->numTestTime);

		gStartTest = TRUE;

		UpdateUeConfig(pPhyUeSim, paramReq->numUe, paramReq->numTestTime);
	} else {
		LOG_TRACE(MODULE_ID_LAYER_MGR, "[%s], Invalid msgId = %d\n", __func__, msgId)
	}

}

void TestTask(void)
{
	int i = 1;
	while (1) {
		i++;
	}
}

Int32 InitSimulator()
{
	Int32 i = 0;
    InitUserQueueHandle(&l2LogMsgQueue_g, sizeof(S_LogFormat), (Uint32)LOG_TASK_RX_MSG_NUM, &gLogMsgPtrQueue[0]);
    for(i = 0; i < LOG_TASK_RX_MSG_NUM; i++)
    {
    	gLogMsgPtrQueue[i] = (PTR)&gLogFormatBuf[i];
    }

	/* creat mailbox/queue */
    if(RET_SUCCESS != CreateMsgQueue(&PhySimLogMsgQ_g, (UInt32)0, (UInt32)Phy_Sim_Handler_Msg_Size, (UInt32)Phy_Sim_Handler_Msg_Num))
    {
    	LOG_ERROR(MODULE_ID_LAYER_MGR,"[%s], create PhySimLogMsgQ_g failure.\n", __func__);
    	return RET_FAIL;
    }
    if(RET_SUCCESS != CreateMsgQueue(&PhySimHandlerMsgQ_g, (UInt32)0,(UInt32)Phy_Sim_Handler_Msg_Size,(UInt32)Phy_Sim_Handler_Msg_Num))
    {
    	LOG_ERROR(MODULE_ID_LAYER_MGR,"[%s], create PhySimHandlerMsgQ_g failure\n", __func__);
    	return RET_FAIL;
    }

	/* creat task */
    PhySimLogTask_g = CreateTask((UInt32)LOG_TASK_PRIORITY, (UInt32)LOG_TASK_STACK_SIZE, PhySimLogTaskStack_g, (void*)ExportLogInfo);
	PhySimTask_g = CreateTask((UInt32)UE_SIM_TASK_PRIORITY, (UInt32)UE_SIM_TASK_STACK_SIZE, PhySimTaskStack_g, (void*)UePhySimTask);
//	TestTask_g = CreateTask((UInt32)TEST_TASK_PRIORITY, (UInt32)TEST_TASK_STACK_SIZE, TestTaskStack_g, (void*)TestTask);

	/* active task */
	if(RET_SUCCESS != ActiveTask(&PhySimLogTask_g))
	{
		LOG_ERROR(MODULE_ID_LAYER_MGR,"phySim: [%s] ActiveTask PhySimLogTask_g failure.\n", __func__);
	}
	if(RET_SUCCESS != ActiveTask(&PhySimTask_g))
	{
		LOG_ERROR(MODULE_ID_LAYER_MGR,"phySim: [%s] ActiveTask PhySimTask_g failure.\n", __func__);
	}


	return RET_SUCCESS;
}

} // extern "C"

#endif
