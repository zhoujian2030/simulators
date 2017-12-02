/*
 * UeService.h
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifndef UE_SERVICE_H
#define UE_SERVICE_H

#ifdef OS_LINUX

#include "Service.h"
#include "EventIndicator.h"
#include "UdpSocket.h"
#include "SfnSfManager.h"
#include "SelectSocketSet.h"
#include "PhyMacAPI.h"

namespace ue {

    // Initialize a UE service to simulate mutiple UE 
    class UeService : public cm::Service {
    public:
        UeService(std::string serviceName);
        virtual ~UeService();

        void postEvent();

    private:
        virtual unsigned long run();
        
        cm::EventIndicator m_msEvent;

        net::UdpSocket* m_udpServerSocket;
        UInt8 m_recvBuffer[SOCKET_BUFFER_LENGTH];

        net::SelectSocketSet* m_selectSocketSet;
        PhyMacAPI* m_phyMacAPI;
    };

    inline void UeService::postEvent() {
        m_msEvent.set();
    }

}

#else

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include "userQueue.h"

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
#define LOG_TASK_RX_MSG_NUM   4096
#define LOG_MGR_MSG_PTR_SIZE  sizeof(S_LogFormat)

#define LOG_MGR_MSG_NUM       1024
#define LOG_MGR_MSG_SIZE 	  (4 * 1024)

PTR gLogMsgPtrQueue[LOG_TASK_RX_MSG_NUM];
S_LogFormat gLogFormatBuf[LOG_TASK_RX_MSG_NUM];
S_UserQueueHandle l2LogMsgQueue_g;
/***********************log**************************/

extern void handleCliCommand(void* pPhyUeSim, UInt8* pBuff, UInt32 length);

#define LTE_GET_U16BIT(p_buff)                                        \
    ((UInt16)(*(p_buff) << 8) |                                        \
        (UInt16)(*(p_buff + 1)))

// module id
#define KPI_MODULE_ID            0x9901
#define CLI_MODULE_ID            0x9902
#define SIM_MODULE_ID            0x9903
#define MAC_MODULE_ID            7
#define RRC_MODULE_ID            3
#define RLC_MODULE_ID            6
#define OAM_MODULE_ID            1
#define PDCP_MODULE_ID           5
#define PACKET_RELAY_MODULE_ID   4
#define RRM_MODULE_ID            2
#define PHY_MODULE_ID            8

typedef enum {
    MAC_CLI_SET_LOG_LEVEL_REQ = 0x01,
    MAC_CLI_SET_COMM_CHAN_RAT2 = 0x02,
    MAC_CLI_SET_RACH_THRESTHOLD = 0x03,

    SIM_CLI_SET_PARAM_REQ = 0x11
} CLIReqAPI;

typedef struct {
    UInt32 numUe;
    UInt32 numTestTime;
} SetSIMParamReq;

#define CLI_MSG_HEAD_LENGTH 12

#ifdef __cplusplus
}
#endif

#endif

#endif
