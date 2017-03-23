
#ifndef _LAYER_MGR_H_
#define _LAYER_MGR_H_
#ifdef __cplusplus
extern "C" {
#endif

#ifndef OS_LINUX

#include "../osAdapter/common/platform.h"
#include "../common/userQueue.h"

/***************************task*******************************/
/* task */
TaskHandle PhySimTask_g;
TaskHandle PhySimLogTask_g;

/* task priority */
#define Phy_Sim_Task_Priority 1
#define Phy_Sim_Log_Task_Priority 2

/* task stack size */
#define Phy_Sim_Task_Stack_Size (256*1024)
UInt8	PhySimTaskStack_g[Phy_Sim_Task_Stack_Size];

#define Phy_Sim_Log_Task_Stack_Size (256*1024)
UInt8	PhySimLogTaskStack_g[Phy_Sim_Log_Task_Stack_Size];

/***************************task*******************************/

/***********************mailbox/queue**************************/
MsgQueueHandle PhySimHandlerMsgQ_g;
MsgQueueHandle PhySimLogMsgQ_g;

/* mailbox/queue num */
#define Phy_Sim_Handler_Msg_Num 4

/* mailbox/queue size */
#define Phy_Sim_Handler_Msg_Size 4
/***********************mailbox/queue**************************/

/***********************log**************************/
#define LOG_TASK_RX_MSG_NUM   4096 /* L2�ڲ�LOG�߳�LOG���пɽ��յ����LOG���� */
#define LOG_MGR_MSG_PTR_SIZE  sizeof(S_LogFormat)

#define LOG_MGR_MSG_NUM       1024/* ���������ṩ����32 */
#define LOG_MGR_MSG_SIZE 	  (4 * 1024)

PTR gLogMsgPtrQueue[LOG_TASK_RX_MSG_NUM]; /* �ڲ����LOG����  */
S_LogFormat gLogFormatBuf[LOG_TASK_RX_MSG_NUM]; /* �ڲ����LOG���� */
S_UserQueueHandle l2LogMsgQueue_g;/* �ڲ�����LOG���� */
/***********************log**************************/

#endif

#ifdef __cplusplus
}
#endif

#endif/*_LAYER_MGR_H_*/
