/*
 * UeScheduler.cpp
 *
 *  Created on: Nov 07, 2016
 *      Author: j.zhou
 */



#include "UeScheduler.h"
#include "UeTerminal.h"
#include "UeMacAPI.h"
#include "CLogger.h"

using namespace ue;
using namespace std;

// ----------------------------------------
UeScheduler::UeScheduler(UeMacAPI* ueMacAPI) 
: m_sfn(0), m_sf(0)
{
    // The ueId and ra-rnti value is in range 1~MAC_UE_SUPPORTED
    m_ueList = new UeTerminal*[MAC_UE_SUPPORTED];
    for (UInt32 i=0; i<MAC_UE_SUPPORTED; i++) {
        m_ueList[i] = new UeTerminal(i+1, i+1, ueMacAPI);
    }

    for (UInt32 i=0; i<DL_MSG_CONTAINER_SIZE; i++) {
        m_dlMsgBufferContainer[i].length = 0;
    }
    m_ulCfgMsg.length = 0;

    m_nodePool = new NodePool(DL_MSG_CONTAINER_SIZE);
}

// ----------------------------------------
UeScheduler::~UeScheduler() {

}

// ----------------------------------------
void UeScheduler::updateSfnSf(UInt16 sfn, UInt8 sf) {
    m_sfn = sfn;
    m_sf  = sf;
}

// ----------------------------------------
#define GENERATE_SUBFRAME_SFNSF(sfn,sf) ( ( (sfn) << 4) | ( (sf) & 0xf) )
void UeScheduler::processDlData(UInt8* buffer, SInt32 length) {
    LOG_DEBUG(UE_LOGGER_NAME, "Entry \n");

    FAPI_l1ApiMsg_st *pL1Api = (FAPI_l1ApiMsg_st *)buffer;

    if (pL1Api->msgId == PHY_UL_CONFIG_REQUEST) {
        memcpy(m_ulCfgMsg.data, buffer, length);
        m_ulCfgMsg.length = length;
        LOG_DEBUG(UE_LOGGER_NAME, "save PHY_UL_CONFIG_REQUEST, in %d.%d\n", m_sfn, m_sf);
    } else {
        if (pL1Api->msgId == PHY_DELETE_UE_REQUEST) {
            UInt16* pSfnSf = (UInt16*)&pL1Api->msgBody[0];
            UInt8 sf = (m_sf + 2) % 10;
            UInt16 sfn = (m_sfn + (m_sf + 2)/10) % 1024;
            *pSfnSf = GENERATE_SUBFRAME_SFNSF(sfn, sf);
            LOG_DEBUG(UE_LOGGER_NAME, "recv PHY_DELETE_UE_REQUEST in %d.%d, will handle it in %d.%d\n", m_sfn, m_sf, sfn, sf);
        }

        Node* node = m_nodePool->getNode();
        if (node == 0) {
            LOG_ERROR(UE_LOGGER_NAME, "Fail to get free node\n");
            return;
        }
        for (UInt32 i=0; i<DL_MSG_CONTAINER_SIZE; i++) {
            if (m_dlMsgBufferContainer[i].length == 0) {
                node->buffer = (void*)&m_dlMsgBufferContainer[i];
                memcpy(m_dlMsgBufferContainer[i].data, buffer, length);
                m_dlMsgBufferContainer[i].length = length;

                if (m_head == 0) {
                    m_head = node;
                    m_head->tail = node;
                } else {
                    m_head->tail->next = node;
                    m_head->tail = node;
                }

                LOG_DEBUG(UE_LOGGER_NAME, "save node = %p, in %d.%d\n", node, m_sfn, m_sf);
                break;
            }
        }  
    }
}

// ----------------------------------------
void UeScheduler::handleDeleteUeReq(UInt16 rnti) {
    LOG_DEBUG(UE_LOGGER_NAME, "delete rnti = %d\n", rnti);

    if (rnti <= m_maxRaRntiUeId) {
        // For RAR, rnti is ra-rnti, which is the same as ueId
        m_ueList[rnti-1]->handleDeleteUeReq();
    } else {
        // other msg, c-rnti
        map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
        if (it != m_rntiUeIdMap.end()) {
            UInt8 ueId = it->second;
            if (ueId <= m_maxRaRntiUeId) {
                m_ueList[ueId-1]->handleDeleteUeReq();
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
            }
        } else {
            LOG_ERROR(UE_LOGGER_NAME, "Fail to get ueId by rnti = %d\n", rnti);
        }
    }
}

// ----------------------------------------
void UeScheduler::schedule() {
    m_pduIndexUeIdMap.clear();

    int numUeSchedule = 3;//MAC_UE_SUPPORTED;
    for(int i=0; i<numUeSchedule; i++) {
        m_ueList[i]->schedule(m_sfn, m_sf, this);
    }

    processData();
}

// ----------------------------------------
BOOL UeScheduler::validateSfnSf(BOOL isULCfg, UInt16 sfn, UInt8 sf) {
    UInt8 delay = DL_PHY_DELAY;
    if (isULCfg) {
        delay = UL_PHY_DELAY;
    }
    UInt32 curTick = m_sfn * 10 + m_sf;
    UInt16 provTick = sfn * 10 + sf;
    UInt8 delta;
    if (provTick < curTick) {
        delta = provTick + 10240 - curTick;
    } else {
        delta = provTick - curTick;
    }

    if (delta > delay) {
        return FALSE;
    } else {
        return TRUE;
    }        
}

// ----------------------------------------
void UeScheduler::processData() {
    // process UL Cfg
    if (m_ulCfgMsg.length > 0) {
        FAPI_l1ApiMsg_st* pL1Api = (FAPI_l1ApiMsg_st *)m_ulCfgMsg.data;
        FAPI_ulConfigRequest_st *pUlConfigReq = (FAPI_ulConfigRequest_st *)&pL1Api->msgBody[0];        
        UInt8 sf  = pUlConfigReq->sfnsf & 0x000f;
        UInt16 sfn  = (pUlConfigReq->sfnsf & 0xfff0) >> 4;
        LOG_DEBUG(UE_LOGGER_NAME, "handle msgId = 0x%02x in %d.%d, the provision sfnsf is %d.%d\n", 
            pL1Api->msgId, m_sfn, m_sf, sfn, sf);
 
        if (!validateSfnSf(TRUE, sfn, sf) || (sf == m_sf && sfn == m_sfn)) {
            if(pUlConfigReq->ulConfigLen == (pL1Api->msgLen + pL1Api->lenVendorSpecific)) {
                handleUlConfigReq(pUlConfigReq);
            }
            m_ulCfgMsg.length = 0;
        }
    }

    // process DL Cfg / DL Data
    Node* node = 0;
    DlMsgBuffer* msgBuffer = 0;

    while (m_head != 0) {
        LOG_DEBUG(UE_LOGGER_NAME, "m_head = %p\n", m_head);
        node = m_head;
        msgBuffer = (DlMsgBuffer*)node->buffer;
        UInt16 msgId = 0;
        FAPI_l1ApiMsg_st *pL1Api = (FAPI_l1ApiMsg_st *)msgBuffer->data;
        msgId = pL1Api->msgId;       

        FapiL1MsgHead* pMsgHead = (FapiL1MsgHead*)&pL1Api->msgBody[0];
        UInt8 sf  = pMsgHead->sfnsf & 0x000f;
        UInt16 sfn  = (pMsgHead->sfnsf & 0xfff0) >> 4;
        LOG_DEBUG(UE_LOGGER_NAME, "handle msgId = 0x%02x in %d.%d, the provision sfnsf is %d.%d\n", 
            msgId, m_sfn, m_sf, sfn, sf);

        if (!validateSfnSf(TRUE, sfn, sf) || (sf == m_sf && sfn == m_sfn)) {
            switch(msgId) {
                case PHY_DELETE_UE_REQUEST:
                {
                    UInt16* pMsg = (UInt16*)&pL1Api->msgBody[0];
                    UInt16* pRnti = ++pMsg;
                    handleDeleteUeReq(*pRnti);
                    break;
                }
                
                case PHY_DL_CONFIG_REQUEST:
                {
                    FAPI_dlConfigRequest_st *pDlConfigReq = (FAPI_dlConfigRequest_st *)&pL1Api->msgBody[0];
                    if (pDlConfigReq->length == (pL1Api->msgLen + pL1Api->lenVendorSpecific)) {
                        handleDlConfigReq(pDlConfigReq);
                    }            
                    break;
                }

                // case PHY_UL_CONFIG_REQUEST:
                // {
                //     FAPI_ulConfigRequest_st *pUlConfigReq = (FAPI_ulConfigRequest_st *)&pL1Api->msgBody[0];
                //     if(pUlConfigReq->ulConfigLen == (pL1Api->msgLen + pL1Api->lenVendorSpecific)) {
                //         handleUlConfigReq(pUlConfigReq);
                //     }
                //     break;            
                // }
                
                case PHY_DL_HI_DCI0_REQUEST:
                {
                    FAPI_dlHiDCIPduInfo_st *pHIDci0Req = (FAPI_dlHiDCIPduInfo_st *)&pL1Api->msgBody[0];
                    handleHIDci0Req(pHIDci0Req);
                    break;
                }

                case PHY_DL_TX_REQUEST:
                {
                    FAPI_dlDataTxRequest_st* pDlDataReq = (FAPI_dlDataTxRequest_st*)&pL1Api->msgBody[0];
                    handleDlDataReq(pDlDataReq);
                    break;
                }

                default:
                    LOG_ERROR(UE_LOGGER_NAME, "Invalid msgId = 0x%02x\n", msgId);
                    break;
            }  

            // free the buffer and node
            msgBuffer->length = 0;
            m_head = node->next;
            if (m_head !=0 ) {
                m_head->tail = node->tail;
            }
            m_nodePool->freeNode(node);
        } else {
            LOG_DEBUG(UE_LOGGER_NAME, "stop handling\n");
            break;
        }
    }
}

// --------------------------------------------------------
void UeScheduler::resetUeTerminal(UInt16 rnti, UInt8 ueId) {
    LOG_DEBUG(UE_LOGGER_NAME, "rnti = %d, ueId = %d, %d.%d\n", rnti, ueId, m_sfn, m_sf);

    std::map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
    if (it != m_rntiUeIdMap.end()) {
        m_rntiUeIdMap.erase(it);
    }

    it = m_pduIndexUeIdMap.begin();
    while (it != m_pduIndexUeIdMap.end()) {
        if (it->second == ueId) {
            m_pduIndexUeIdMap.erase(it++);
        } else {
            it++;
        }
    }

    // it = m_harqIdUeIdMap.begin();
    // while (it != m_harqIdUeIdMap.end()) {
    //     if (it->second == ueId) {
    //         m_harqIdUeIdMap.erase(it++);
    //     } else {
    //         it++;
    //     }
    // }

    std::vector<UInt32>::iterator iter = m_ueIdHarqIdVect.begin();
    while (iter != m_ueIdHarqIdVect.end()) {
        UInt8 foundUeId = ((*iter) >> 16) & 0xff;
        if (foundUeId == ueId) {
            iter = m_ueIdHarqIdVect.erase(iter);
        } else {
            ++iter;
        }
    }
}

// ----------------------------------------
void UeScheduler::handleDlConfigReq(FAPI_dlConfigRequest_st* pDlConfigReq) {
    UInt32 headerLen = (UInt32)((UInt8*)&pDlConfigReq->dlConfigpduInfo[0] - (UInt8*)pDlConfigReq); 
    if((headerLen == pDlConfigReq->length)
        || ((pDlConfigReq->numDCI + pDlConfigReq->numOfPDU + pDlConfigReq->numOfPDSCHRNTI) <= 0))
    {
        LOG_WARN(UE_LOGGER_NAME, "Empty PHY_DL_CONFIG_REQUEST\n");
        return;
    } 

    UInt8 sf  = pDlConfigReq->sfnsf & 0x000f;
    UInt16 sfn  = (pDlConfigReq->sfnsf & 0xfff0) >> 4;

    LOG_DEBUG(UE_LOGGER_NAME, "Recv PHY_DL_CONFIG_REQUEST, provSfnSf = %d.%d, curSfnSf = %d.%d\n", 
        sfn, sf, m_sfn, m_sf);
    
    FAPI_dlConfigPDUInfo_st *pNextPdu, *pPrevPdu;
    pNextPdu = (FAPI_dlConfigPDUInfo_st *)&pDlConfigReq->dlConfigpduInfo[0];
    Int length = 0;
    length += ((uintptr_t)pNextPdu - (uintptr_t)pDlConfigReq);

    do{
        pPrevPdu = pNextPdu;

        LOG_DEBUG(UE_LOGGER_NAME, "Recv PHY_DL_CONFIG_REQUEST, pduType = %d\n", pNextPdu->pduType);

        switch (pNextPdu->pduType) {
            case FAPI_DCI_DL_PDU:
            {
                UInt16 rnti = pNextPdu->dlConfigpduInfo.DCIPdu.rnti;
                if (pNextPdu->dlConfigpduInfo.DCIPdu.dciFormat == FAPI_DL_DCI_FORMAT_1A ||
                    pNextPdu->dlConfigpduInfo.DCIPdu.dciFormat == FAPI_DL_DCI_FORMAT_1) {
                    if (rnti != 0xffff) {
                        LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_DL_DCI_FORMAT_1A / FAPI_DL_DCI_FORMAT_1 (%d), "
                            "provSfnSf = %d.%d, rnti = %d, curSfnSf = %d.%d\n",
                            pNextPdu->dlConfigpduInfo.DCIPdu.dciFormat, sfn, sf, rnti, m_sfn, m_sf);
                        
                        if (rnti <= m_maxRaRntiUeId) {
                            // For RAR, rnti is ra-rnti, which is the same as ueId
                            m_ueList[rnti-1]->handleDlDciPdu(pDlConfigReq, &pNextPdu->dlConfigpduInfo.DCIPdu);
                        } else {
                            // other msg, c-rnti
                            map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
                            if (it != m_rntiUeIdMap.end()) {
                                UInt8 ueId = it->second;
                                if (ueId <= m_maxRaRntiUeId) {
                                    m_ueList[ueId-1]->handleDlDciPdu(pDlConfigReq, &pNextPdu->dlConfigpduInfo.DCIPdu);
                                } else {
                                    LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
                                }
                            } else {
                                LOG_ERROR(UE_LOGGER_NAME, "Fail to get ueId by rnti = %d\n", rnti);
                            }
                        }

                    } else {
                        LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_DL_DCI_FORMAT_1A (broadcast), "
                            "provSfnSf = %d.%d, rnti = %d, curSfnSf = %d.%d\n",
                            sfn, sf, rnti, m_sfn, m_sf);
                    }

                    pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize); 
                    if (pPrevPdu != pNextPdu) {
                        length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                    }
                } else {
                    LOG_DEBUG(UE_LOGGER_NAME, "provSfnSf = %d.%d, curSfnSf = %d.%d, rnti = %d, dciFormat = %d, TODO\n",
                            sfn, sf, m_sfn, m_sf, rnti, pNextPdu->dlConfigpduInfo.DCIPdu.dciFormat);

                    pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize); 
                    if (pPrevPdu != pNextPdu) {
                        length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                    }
                }
                break;
            }

            case FAPI_DLSCH_PDU:
            {
                UInt16 rnti = pNextPdu->dlConfigpduInfo.DlSCHPdu.rnti;
                UInt16 pduIndex = pNextPdu->dlConfigpduInfo.DlSCHPdu.pduIndex;
                if (rnti != 0xffff) {
                    LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_DLSCH_PDU, "
                        "provSfnSf = %d.%d, rnti = %d, curSfnSf = %d.%d\n",
                        sfn, sf, pNextPdu->dlConfigpduInfo.DlSCHPdu.rnti, m_sfn, m_sf);
                    // TODO further handling according to rnti
                    if (rnti <= m_maxRaRntiUeId) {
                        // For RAR, rnti is ra-rnti, which is the same as ueId
                        m_ueList[rnti-1]->handleDlSchPdu(pDlConfigReq, &pNextPdu->dlConfigpduInfo.DlSCHPdu);

                        // save the pduIndex as key to find ueId
                        pair<map<UInt16, UInt8>::iterator, bool> result = 
                            m_pduIndexUeIdMap.insert(map<UInt16, UInt8>::value_type(pduIndex, rnti));
                        if (!result.second) {
                            (result.first)->second = rnti;
                        }
                    } else {
                        // other msg, c-rnti
                        map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
                        if (it != m_rntiUeIdMap.end()) {
                            UInt8 ueId = it->second;
                            if (ueId <= m_maxRaRntiUeId) {
                                m_ueList[ueId-1]->handleDlSchPdu(pDlConfigReq, &pNextPdu->dlConfigpduInfo.DlSCHPdu);

                                // save the pduIndex as key to find ueId
                                pair<map<UInt16, UInt8>::iterator, bool> result = 
                                    m_pduIndexUeIdMap.insert(map<UInt16, UInt8>::value_type(pduIndex, ueId));
                                if (!result.second) {
                                    (result.first)->second = ueId;
                                }
                            } else {
                                LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
                            }
                        } else {
                            LOG_ERROR(UE_LOGGER_NAME, "Fail to get ueId by rnti = %d\n", rnti);
                        }
                    }
                } else {
                    LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_DLSCH_PDU (broadcast), "
                        "provSfnSf = %d.%d, rnti = %d, curSfnSf = %d.%d\n",
                        sfn, sf, rnti, m_sfn, m_sf);                    
                }

                pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8*)pNextPdu) + pNextPdu->pduSize);
                if (pPrevPdu != pNextPdu) {
                    length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                }
                break;
            }

            case FAPI_PCH_PDU:
            case FAPI_PRS_PDU:
            {
                pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize);
                if (pPrevPdu != pNextPdu) {
                    length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                }
                break;
            }

            case FAPI_BCH_PDU:
            {
                // NOT handle MIB
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_BCH_PDU\n");
                pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize);
                if (pPrevPdu != pNextPdu) {
                    length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                }
                break;
            }

            default:
                LOG_DEBUG(UE_LOGGER_NAME, "Invalid pduType = %d\n", pNextPdu->pduType);
                pNextPdu = (FAPI_dlConfigPDUInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduSize);
                if (pPrevPdu != pNextPdu) {
                    length += ((uintptr_t)pNextPdu - (uintptr_t)pPrevPdu);
                }
                break;
        }        

    } while(length < pDlConfigReq->length);
}

// ----------------------------------------
void UeScheduler::handleUlConfigReq(FAPI_ulConfigRequest_st* pUlConfigReq) {
    UInt32 headerLen = (UInt8*)&pUlConfigReq->ulPduConfigInfo[0] - (UInt8*)pUlConfigReq;

    if((headerLen == pUlConfigReq->ulConfigLen) || ((pUlConfigReq->numOfPdu) <= 0))
    {
        return;
    }

    UInt8 sf  = pUlConfigReq->sfnsf & 0x000f;
    UInt16 sfn  = (pUlConfigReq->sfnsf & 0xfff0) >> 4;

    LOG_DEBUG(UE_LOGGER_NAME, "Recv PHY_UL_CONFIG_REQUEST, provSfnSf = %d.%d, curSfnSf = %d.%d\n", 
        sfn, sf, m_sfn, m_sf);

    UInt8  *tmpBuff = (UInt8 *)(pUlConfigReq->ulPduConfigInfo);
    UInt16 pduSize = 0;
    UInt32 pduIndex=0;
    FAPI_ulPDUConfigInfo_st* ulPduConf_p = PNULL;
    for(; pduIndex < pUlConfigReq->numOfPdu; pduIndex++) {
        ulPduConf_p = (FAPI_ulPDUConfigInfo_st *)(tmpBuff + pduSize);

        switch(ulPduConf_p->ulConfigPduType) {                    
            case FAPI_ULSCH:
            {
                FAPI_ulSCHPduInfo_st *pUlSchPdu = (FAPI_ulSCHPduInfo_st *)(ulPduConf_p->ulPduConfigInfo);
                LOG_DEBUG(UE_LOGGER_NAME, "[%d.%d], Recv FAPI_ULSCH, rnti = %d, numOfRB = %d, provSfnSf = %d.%d\n",
                    m_sfn, m_sf, pUlSchPdu->rnti, pUlSchPdu->numOfRB, sfn, sf);

                map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(pUlSchPdu->rnti);
                if (it != m_rntiUeIdMap.end()) {
                    UInt8 ueId = it->second;
                    if (ueId <= m_maxRaRntiUeId) {
                        m_ueList[ueId-1]->handleUlSchPdu(pUlConfigReq, pUlSchPdu);
                    } else {
                        LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
                    }
                } else {
                    LOG_ERROR(UE_LOGGER_NAME, "Fail to get ueId by rnti = %d\n", pUlSchPdu->rnti);
                }

                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_ULSCH_HARQ:
            {
                FAPI_ulSCHHarqPduInfo_st* ulschHarq_p = (FAPI_ulSCHHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_ULSCH_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    ulschHarq_p->ulSCHPduInfo.rnti, sfn, sf, m_sfn, m_sf);

                map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(ulschHarq_p->ulSCHPduInfo.rnti);
                if (it != m_rntiUeIdMap.end()) {
                    UInt8 ueId = it->second;
                    if (ueId <= m_maxRaRntiUeId) {
                        m_ueList[ueId-1]->handleUlSchPdu(pUlConfigReq, &ulschHarq_p->ulSCHPduInfo);
                    } else {
                        LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
                    }
                } else {
                    LOG_ERROR(UE_LOGGER_NAME, "Fail to get ueId by rnti = %d\n", ulschHarq_p->ulSCHPduInfo.rnti);
                }

                // TODO count the ULSCH_HARQ msg 

                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_ULSCH_CQI_RI:
            {
                FAPI_ulSCHCqiRiPduInfo_st* ulschCqiRi_p = (FAPI_ulSCHCqiRiPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_ULSCH_CQI_RI, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    ulschCqiRi_p->ulSCHPduInfo.rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_ULSCH_CQI_HARQ_RI:
            {
                FAPI_ulSCHCqiHarqRIPduInfo_st* ulschCqiHarqRi_p = (FAPI_ulSCHCqiHarqRIPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_ULSCH_CQI_HARQ_RI, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    ulschCqiHarqRi_p->ulSCHPduInfo.rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_SR:
            {
                FAPI_uciSrPduInfo_st* uciSrPdu_p = (FAPI_uciSrPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_UCI_SR, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    uciSrPdu_p->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_HARQ:
            {
                FAPI_uciHarqPduInfo_st *uciHarq_p = (FAPI_uciHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_UCI_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    uciHarq_p->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_SR_HARQ:
            {
                FAPI_uciSrHarqPduInfo_st *uciSrHarq = (FAPI_uciSrHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_UCI_SR_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    uciSrHarq->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_CQI:
            {
                FAPI_uciCqiPduInfo_st *uciCqi = (FAPI_uciCqiPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_UCI_CQI, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    uciCqi->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_CQI_HARQ:
            {
                FAPI_uciCqiHarqPduInfo_st *uciCqiHarq = (FAPI_uciCqiHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_UCI_CQI_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    uciCqiHarq->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_CQI_SR:
            {
                FAPI_uciCqiSrPduInfo_st *uciCqiSr = (FAPI_uciCqiSrPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_UCI_CQI_SR, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    uciCqiSr->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_UCI_CQI_SR_HARQ:
            {
                FAPI_uciCqiSrHarqPduInfo_st *uciCqiSrHarq = (FAPI_uciCqiSrHarqPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_UCI_CQI_SR_HARQ, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    uciCqiSrHarq->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }

            case FAPI_SRS:
            {
                FAPI_srsPduInfo_st * srsPdu_p = (FAPI_srsPduInfo_st *)ulPduConf_p->ulPduConfigInfo;
                LOG_DEBUG(UE_LOGGER_NAME, "Recv FAPI_SRS, rnti = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    srsPdu_p->rnti, sfn, sf, m_sfn, m_sf);
                pduSize += ulPduConf_p->ulConfigPduSize;
                break;
            }
        
            default:
            {
                LOG_WARN(UE_LOGGER_NAME, "Recv ulConfigPduType = %d, "
                    "provSfnSf = %d.%d, curSfnSf = %d.%d\n",
                    ulPduConf_p->ulConfigPduType, sfn, sf, m_sfn, m_sf);
                break;
            }
        }
    }                  
}

// ----------------------------------------
void UeScheduler::handleHIDci0Req(FAPI_dlHiDCIPduInfo_st* pHIDci0Req) {
    // UInt32 headerLen = (UInt8*)&pHIDci0Req->hidciPduInfo[0] - (UInt8*)pHIDci0Req;
    if((pHIDci0Req->numOfDCI + pHIDci0Req->numOfHI) <= 0) {
        return;
    }

    UInt8 sf  = pHIDci0Req->sfnsf & 0x000f;
    UInt16 sfn  = (pHIDci0Req->sfnsf & 0xfff0) >> 4;
    LOG_DEBUG(UE_LOGGER_NAME, "Recv PHY_DL_HI_DCI0_REQUEST, provSfnSf = %d.%d, curSfnSf = %d.%d\n",
        sfn, sf, m_sfn, m_sf);

    UInt8* pNextPdu = PNULL;
    UInt8* pPrevPdu = PNULL;
    
    pNextPdu = &(pHIDci0Req->hidciPduInfo[0]);

    UInt8 numOfDCI = 0;
    UInt8 numOfHI = 0;

    while((numOfDCI < pHIDci0Req->numOfDCI) || (numOfHI < pHIDci0Req->numOfHI)) {
        pPrevPdu = pNextPdu;
        UInt8 pduType = *((UInt8*)pNextPdu);
        if (pduType == FAPI_HI_PDU) { 
            // HARQ ACK for UL TB
            FAPI_dlHiPduInfo_st* pHiPdu = (FAPI_dlHiPduInfo_st*)pNextPdu;
            LOG_DEBUG(UE_LOGGER_NAME,"Recv FAPI_HI_PDU, rbStart = %d, cyclicShift2_forDMRS = %d, hiValue = %d, iPHICH = %d, txPower = %d\n", 
                pHiPdu->rbStart, pHiPdu->cyclicShift2_forDMRS, pHiPdu->hiValue, pHiPdu->iPHICH, pHiPdu->txPower);

            UInt32 ueIdHarqId;
            UInt8 ueId;
            UInt16 harqId = (pHiPdu->rbStart << 8) | pHiPdu->cyclicShift2_forDMRS;
            vector<UInt32>::iterator it = m_ueIdHarqIdVect.begin();
            while(it!=m_ueIdHarqIdVect.end()) {
                ueIdHarqId = *it;
                LOG_DEBUG(UE_LOGGER_NAME, "ueIdHarqId = %04x\n", ueIdHarqId);
                if ((ueIdHarqId & 0xffff) == harqId) {
                    ueId = (ueIdHarqId >> 16) & 0xff;
                    if (ueId <= m_maxRaRntiUeId) {
                        if (m_ueList[ueId-1]->handleHIPdu(pHIDci0Req, pHiPdu)) {
                            LOG_DEBUG(UE_LOGGER_NAME, "Handle harq ack success\n");
                            m_ueIdHarqIdVect.erase(it);
                            break;
                        } else {
                            LOG_INFO(UE_LOGGER_NAME, "Handle harq ack failed in ueId = %d\n", ueId);
                        }
                    } else {
                        LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
                    }
                }

                ++it;
            }

            // UInt16 harqId = (pHiPdu->rbStart << 8) | pHiPdu->cyclicShift2_forDMRS;
            // map<UInt16, UInt8>::iterator it = m_harqIdUeIdMap.find(harqId);
            // if (it != m_harqIdUeIdMap.end()) {
            //     UInt8 ueId = it->second;
            //     if (ueId <= m_maxRaRntiUeId) {
            //         m_ueList[ueId-1]->handleHIPdu(pHIDci0Req, pHiPdu);

            //         // TBD. really need to delete it?
            //         m_harqIdUeIdMap.erase(it);
            //     } else {
            //         LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
            //     }
            // } else {
            //     LOG_ERROR(UE_LOGGER_NAME, "Fail to get ueId by harqId = %d\n", harqId);
            // }

            numOfHI++;
            pNextPdu = ((UInt8 *)pNextPdu) + sizeof(FAPI_dlHiPduInfo_st);
 
        } else if (pduType == FAPI_DCI_UL_PDU) {
            FAPI_dlDCIPduInfo_st* pDciPdu = (FAPI_dlDCIPduInfo_st*)pNextPdu;
            UInt16 rnti = pDciPdu->rnti;          
            map<UInt16, UInt8>::iterator it = m_rntiUeIdMap.find(rnti);
            if (it != m_rntiUeIdMap.end()) {
                UInt8 ueId = it->second;
                if (ueId <= m_maxRaRntiUeId) {
                    m_ueList[ueId-1]->handleDci0Pdu(pHIDci0Req, pDciPdu);

                    UInt32 ueIdHarqId = (ueId << 16) | (pDciPdu->rbStart << 8) | pDciPdu->cyclicShift2_forDMRS;
                    m_ueIdHarqIdVect.push_back(ueIdHarqId);

                    // // save the harqId as key to find ueId
                    // UInt16 harqId = (pDciPdu->rbStart << 8) | pDciPdu->cyclicShift2_forDMRS;
                    // pair<map<UInt16, UInt8>::iterator, bool> result = 
                    //     m_harqIdUeIdMap.insert(map<UInt16, UInt8>::value_type(harqId, ueId));
                    // if (!result.second) {
                    //     LOG_WARN(UE_LOGGER_NAME, "harqId 0x%04x record exists, overide it\n", harqId);
                    //     (result.first)->second = ueId;
                    // }
                } else {
                    LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
                }
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "Fail to get ueId by rnti = %d\n", rnti);
            }

            numOfDCI++;
            pNextPdu = ((UInt8 *)pNextPdu) + pDciPdu->ulDCIPDUSize;
        } else {
            LOG_ERROR(UE_LOGGER_NAME, "Invalid pduType = %d\n", pduType);
            break;
        }
    }
}

// ----------------------------------------
void UeScheduler::handleDlDataReq(FAPI_dlDataTxRequest_st* pDlDataReq) {

    UInt8 sf  = pDlDataReq->sfnsf & 0x000f;
    UInt16 sfn  = (pDlDataReq->sfnsf & 0xfff0) >> 4;

    if (pDlDataReq->numOfPDU == 0) {
        LOG_WARN(UE_LOGGER_NAME, "no data pdu received\n");
        return;
    }

    LOG_DEBUG(UE_LOGGER_NAME, "Recv PHY_DL_TX_REQUEST, provSfnSf = %d.%d, curSfnSf = %d.%d\n", 
        sfn, sf, m_sfn, m_sf);
    
    FAPI_dlTLVInfo_st *pDlTlv = PNULL;
    FAPI_dlPduInfo_st *pNextPdu = PNULL;
    FAPI_dlPduInfo_st *pPrevPdu = PNULL;
    pNextPdu = (FAPI_dlPduInfo_st *)&pDlDataReq->dlPduInfo[0];
    Int length = 0;
    length += ((uintptr_t)pNextPdu - (uintptr_t)pDlDataReq);

    Int i;
    for(i = 0; i < pDlDataReq->numOfPDU; i++) {
        pPrevPdu = pNextPdu;
        if (pNextPdu->numOfTLV != 1) {
            LOG_WARN(UE_LOGGER_NAME, "Invalid numOfTLV = %d\n", pNextPdu->numOfTLV);
            break;
        }
        pDlTlv = (FAPI_dlTLVInfo_st *)&pNextPdu->dlTLVInfo[0];
        if (pDlTlv->tag != 0) {
            LOG_WARN(UE_LOGGER_NAME, "Invalid tag = %d\n", pDlTlv->tag);
            break;
        }

        UInt16 pduIndex = pNextPdu->pduIndex;
        map<UInt16, UInt8>::iterator it = m_pduIndexUeIdMap.find(pduIndex);
        if (it != m_pduIndexUeIdMap.end()) {
            UInt8 ueId = it->second;
            if (ueId <= m_maxRaRntiUeId) {
                m_ueList[ueId-1]->handleDlTxData(pDlDataReq, pDlTlv, this);
            } else {
                LOG_ERROR(UE_LOGGER_NAME, "Invalid ueId = %d\n", ueId);
            }
        } else {
            LOG_ERROR(UE_LOGGER_NAME, "Fail to get ueId by pduIndex = %d, it could be BCH pdu\n", pduIndex);
        }

        pNextPdu = (FAPI_dlPduInfo_st *)(((UInt8 *)pNextPdu) + pNextPdu->pduLen);
    }
    
}
