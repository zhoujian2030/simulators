/*
 * UeMain.cpp
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifdef OS_LINUX

#include "UeService.h"
#include "SfnSfManager.h"
#include "StsCounter.h"

UInt8 gLogLevel = 2;

using namespace ue;

int main(int argc, char* argv[]) {

    UeService* service = new UeService("UE Mock");
    // cm::Thread::sleep(2000);
    StsCounter::getInstance();
    SfnSfManager::getInstance()->registerService(service);
    SfnSfManager::getInstance()->start();
    service->wait();

    delete service;

    return 0;
}

#else

#ifdef TI_DSP

extern "C" {

#include <stdio.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/drv/cppi/cppi_osal.h>
#include <ti/drv/qmss/qmss_osal.h>
#include <ti/drv/qmss/qmss_drv.h>
/* CPPI LLD include */
#include <ti/drv/cppi/cppi_drv.h>
#include <ti/drv/cppi/cppi_desc.h>

#include <ti/csl/csl_cacheAux.h>
#include <xdc/cfg/global.h>
#include <pa/qos.h>
#include "qmss.h"
//#include "../osAdapter/sapa/sa.h"
//#include "../osAdapter/sapa/pa.h"
#include "pktQueue.h"
#include "sfnMgr.h"
#include "osCommon.h"
#include "logger.h"
#include "global.h"
#include "xmc_mpu.h"
#include "lteTypes.h"

UInt8 gIpDscpAndEcn = 0xba;
extern S_SysSfn  gSysSfn;
extern MsgQueueHandle PhySimLogMsgQ_g;
extern MsgQueueHandle PhySimHandlerMsgQ_g;
UInt16 gLogTimeoutCount = 0;

/*
 *
 * GetAirSfnIndex
 *
 */
static inline UInt16 GetAirSfnIndex(void)
{
	UInt16 value;

	volatile UInt32* pRadtSymcnReg = (UInt32*)0x01f48050;
	value= (UInt16)((*pRadtSymcnReg) & 0x000003FF);//0~1024
	return value;
}

/*
 *
 * GetAirNsIndex
 *
 */
static inline UInt8 GetAirNsIndex(void)
{
	UInt8 value;

	volatile UInt32* pRadtSymcnReg = (UInt32*)0x01f4804c;
	value = (UInt8)(((*pRadtSymcnReg) >> 19) & 0x000000FF);//0~19
	return value;
}

/*
 *
 * RealTimeCheckIsr
 *
 */
void RealTimeCheckIsr(void)
{

}

/*
 *
 * IsrCmacOamTimer
 *
 */
void IsrCmacOamTimer(void)
{

}

/*
 *
 * ipc_isr
 *
 */
extern volatile BOOL gSfnSfUpdated;
void ipc_isr(void)
{
	UInt32 NoSlot;

	UInt8 expectedSf = (gSysSfn.sf + 1) % 10;
	UInt16 expectedSfn = (gSysSfn.sfn + (gSysSfn.sf + 1) / 10) % 1024;

	gSysSfn.sfn = GetAirSfnIndex();
	NoSlot      = GetAirNsIndex();
	gSysSfn.sf  = NoSlot >> 1;
	gSfnSfUpdated = TRUE;

	UpdateSysTime();

	if (expectedSfn != gSysSfn.sfn || expectedSf != gSysSfn.sf) {
		LOG_ERROR(MODULE_ID_LAYER_MGR,"phySim: SFN/SF not continued\n");
	}

//	LOG_ERROR(MODULE_ID_LAYER_MGR,"phySim: receive one isr...\n");


	UInt32 logTrigger_g = 0xAABBCCDD;

	Mailbox_post(PhySimHandlerMsgQ_g.mbxHandle, &logTrigger_g, BIOS_NO_WAIT);

	if (gLogTimeoutCount == 100) {
		gLogTimeoutCount = 0;
		logTrigger_g = 0;
	} else {
		gLogTimeoutCount++;
	}

	Mailbox_post(PhySimLogMsgQ_g.mbxHandle, &logTrigger_g, BIOS_NO_WAIT);
}

Qmss_QueueHnd lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_MAX];
Qmss_QueueHnd lteLayer2QmssTxHandTable_g[QMSS_TX_HAND_MAX];
Qmss_QueueHnd lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_MAX];

/*
 *
 * qmss init
 *
 */
UInt8 IntiQmss(void)
{
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2D_TO_PHY_DATA]          = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L2][TX_L2_TO_L1_DATA];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2D_TO_PHY_CFG]           = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L2][TX_L2_TO_L1_CONFIG];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2D_TO_L3_DATA_OR_CFG]    = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L2][TX_L2_TO_L3_DATA_OR_CONFIG];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2D_TO_CMAC_BUF_REP]      = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L2][TX_L2_TO_CMAC_BUF_REP];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2D_TO_CMAC_MAC_CE]       = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L2][TX_L2_TO_CMAC_UL_MAC_CE];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2D_TO_OAM_LOG]           = qmssShareObj.Qmss_LogTxFreeQueHnd[TxQue_Type_L2_LOG];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2D_TO_OAM_DATA_OR_REPLY] = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L2][TX_L2_TO_OAM_DATA_OR_REPLY];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2C_TO_LAYER2D_HARQ_ACK]  = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_CMAC][TX_CMAC_TO_L2_HARQ_ACK];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2C_TO_LAYER2D_SCH_RESULT] = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_CMAC][TX_CMAC_TO_L2_SCH_RESULT];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2C_TO_L3_CFG_RSP]        = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_CMAC][TX_CMAC_TO_L3_CFG_RSP];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2C_TO_PHY_CFG]           = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_CMAC][TX_CMAC_TO_L1_CFG];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2C_TO_PHY_DCI]           = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_CMAC][TX_CMAC_TO_L1_DCI];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2C_TO_OAM_LOG]           = qmssShareObj.Qmss_LogTxFreeQueHnd[TxQue_Type_CMAC_LOG];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_LAYER2C_TO_OAM_CFG_RSP]       = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_CMAC][TX_CMAC_TO_OAM_CFG_RSP];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_PHY_TO_LAYER2D_DATAUP]        = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L1][TX_L1_TO_L2_DATAUP];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_PHY_TO_LAYER2D_ERRIND]        = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L1][TX_L1_TO_L2_ERRINDICATION];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_PHY_TO_LAYER2C_CQIUP]         = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L1][TX_L1_TO_CMAC_CQIUP];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_PHY_TO_LAYER2C_REPLY]         = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L1][TX_L1_TO_CMAC_REPLY];
	lteLayer2QmsstxFreeHandTable_g[QMSS_TX_FREE_HAND_PHY_TO_OAM_DATA_OR_REPLY]     = qmssShareObj.Qmss_TxFreeQueHnd[TxQue_Type_L1][TX_L1_TO_OAM_DATAUP_OR_REPLY];

	lteLayer2QmssTxHandTable_g[QMSS_TX_HAND_LAYER2D_TO_OTHER_DATA]                = qmssShareObj.Qmss_TxQueHnd[TxQue_Type_L2];
	lteLayer2QmssTxHandTable_g[QMSS_TX_HAND_LAYER2D_TO_OTHER_LOG]                 = qmssShareObj.Qmss_LogTxQueHnd[TxQue_Type_L2_LOG];
	lteLayer2QmssTxHandTable_g[QMSS_TX_HAND_LAYER2C_TO_OTHER_DATA]                = qmssShareObj.Qmss_TxQueHnd[TxQue_Type_CMAC];
	lteLayer2QmssTxHandTable_g[QMSS_TX_HAND_LAYER2C_TO_OTHER_LOG]                 = qmssShareObj.Qmss_LogTxQueHnd[TxQue_Type_CMAC_LOG];
	lteLayer2QmssTxHandTable_g[QMSS_TX_HAND_PHY_TO_OTHER]                         = qmssShareObj.Qmss_TxQueHnd[TxQue_Type_L1];

	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2D_FROM_CMAC_SCH_RESULT]         = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_L2][RX_L2_FROM_CMAC_SCH_RESULT];
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2D_FROM_CMAC_CMAC_HARQ_ACK]      = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_L2][RX_L2_FROM_CMAC_HARQ_ACK];
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2D_FROM_L1_DATA]                 = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_L2][RX_L2_FROM_L1_DATA];
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2D_FROM_L1_ERRINDICATION]        = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_L2][RX_L2_FROM_L1_ERRINDICATION];
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2D_FROM_L3_DATA_OR_CONFIG]       = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_L2][RX_L2_FROM_L3_DATA_OR_CONFIG];//rrm
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2D_FROM_L3_OAM_CONFIG]           = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_L2][RX_L2_FROM_OAM_CONFIG];//oam,rrc-->mac
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2C_FROM_PHY_CFG_RSP]             = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_CMAC][RX_CMAC_FROM_L1_CFG_RSP];
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2C_FROM_PHY_UL_CQI]              = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_CMAC][RX_CMAC_FROM_L1_UL_CQI];
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2C_FROM_LAYER2D_UL_MAC_CE]       = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_CMAC][RX_CMAC_FROM_L2_UL_MAC_CE];
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2C_FROM_L2_BUF_REP]              = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_CMAC][RX_CMAC_FROM_L2_BUF_REP];
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2C_FROM_L3_CFG_REQ]              = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_CMAC][RX_CMAC_FROM_L3_CFG_REQ];//oam-->rlc
	lteLayer2QmssRxHandTable_g[QMSS_RX_HAND_LAYER2C_FROM_OAM_CFG_REQ]             = qmssShareObj.Qmss_RxQueHnd[TxQue_Type_CMAC][RX_CMAC_FROM_OAM_CFG_REQ];

	return RET_SUCCESS;
}

/*
 *
 * main()
 *
 */
//#define CCS_SIM
#ifndef CCS_SIM
extern Int32 InitSimulator();
SInt32 UeMain(SInt32 argc,char* argv[])
{
	SInt32  isQmssInit = 0;

	do{
		isQmssInit = NAV_syncToMaster();
	}while(isQmssInit !=1);

	if (Qmss_start() != QMSS_SOK)
	{
		printf("L2 DSP, Error starting Queue Manager\n");
	}

	IntiQmss();

	if(RET_FAIL == InitSimulator())
	{
		printf("L2 DSP, initSimulator Fail \n");
	}

	BIOS_start();

    return 0;
}

#else
extern void MainEntryFun();
SInt32 UeMain(SInt32 argc,char* argv[]) {
	MainEntryFun();
	return 0;
}
#endif

} // extern "C"

#endif // TI_DSP


#endif  // OS_LINUX
