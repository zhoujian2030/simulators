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
#define LOG_TASK_RX_MSG_NUM   4096 /* L2ÄÚ²¿LOGÏß³ÌLOG¶ÓÁÐ¿É½ÓÊÕµÄ×î´óLOGÌõÊý */
#define LOG_MGR_MSG_PTR_SIZE  sizeof(S_LogFormat)

#define LOG_MGR_MSG_NUM       1024/* Çý¶¯ÕæÕýÌá¹©µÄÊÇ32 */
#define LOG_MGR_MSG_SIZE 	  (4 * 1024)

PTR gLogMsgPtrQueue[LOG_TASK_RX_MSG_NUM]; /* ÄÚ²¿´æ·ÅLOG¶ÓÁÐ  */
S_LogFormat gLogFormatBuf[LOG_TASK_RX_MSG_NUM]; /* ÄÚ²¿´æ·ÅLOG»º´æ */
S_UserQueueHandle l2LogMsgQueue_g;/* ÄÚ²¿·¢ËÍLOG¶ÓÁÐ */
/***********************log**************************/

#ifdef __cplusplus
}
#endif

#endif

#endif
