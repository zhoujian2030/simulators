/*
 * UeScheduler.h
 *
 *  Created on: Nov 07, 2016
 *      Author: j.zhou
 */

#ifndef UE_SCHEDULER_H
#define UE_SCHEDULER_H

#include "FapiInterface.h"
#include <map>

#define MAC_UE_SUPPORTED 50
#define DL_MSG_CONTAINER_SIZE 3 * 3

namespace ue {

    class UeTerminal;
    class UeMacAPI;

    class UeScheduler {
    public:
        UeScheduler(UeMacAPI* ueMacAPI);
        ~UeScheduler();

        void updateSfnSf(UInt16 sfn, UInt8 sf);

        void processDlData(UInt8* buffer, SInt32 length);
        void handleDlConfigReq(FAPI_dlConfigRequest_st* pDlConfigReq);
        void handleUlConfigReq(FAPI_ulConfigRequest_st* pUlConfigReq);
        void handleHIDci0Req(FAPI_dlHiDCIPduInfo_st* pHIDci0Req);
        void handleDlDataReq(FAPI_dlDataTxRequest_st* pDlDataReq);

        void schedule();
        void processData();

        // callback function by UeTerminal
        void updateRntiUeIdMap(UInt16 rnti, UInt8 ueId);
        void resetUeTerminal(UInt16 rnti, UInt8 ueId);

    private:
        static const UInt16 m_maxRaRntiUeId = MAC_UE_SUPPORTED;
        UInt16 m_sfn;
        UInt8  m_sf;
        UeTerminal** m_ueList;
        std::map<UInt16, UInt8> m_rntiUeIdMap;
        std::map<UInt16, UInt8> m_pduIndexUeIdMap;
        std::map<UInt16, UInt8> m_harqIdUeIdMap;

        struct FapiL1MsgHead {
            UInt16 sfnsf;
        };

        struct DlMsgBuffer {
            UInt8 data[SOCKET_BUFFER_LENGTH];
            SInt32 length;
        };

        struct Node {
            void* buffer;
            Node* next;
            Node* tail;
        };

        class NodePool {
        public:
            NodePool(UInt32 size) {
                m_head = new Node();
                if (size > 0) {
                    size--;
                }

                while (size--) {
                    Node* node = new Node();
                    node->next = m_head;
                    m_head = node;
                }
            }
            ~NodePool() {

            }

            Node* getNode() {
                Node* retNode = m_head;
                if (m_head != 0) {
                    m_head = m_head->next;
                    retNode->next = 0;
                    retNode->tail = 0;
                }
                return retNode;
            }

            void freeNode(Node* node) {
                if (node != 0) {
                    node->next = m_head;
                    m_head = node;
                }
            }

        private:
            Node* m_head;
        };

        DlMsgBuffer m_dlMsgBufferContainer[DL_MSG_CONTAINER_SIZE]; // only support 1 or 2 tick advance
        NodePool* m_nodePool;
        Node* m_head;

        DlMsgBuffer m_ulCfgMsg;
    };

    inline void UeScheduler::updateRntiUeIdMap(UInt16 rnti, UInt8 ueId) {
        m_rntiUeIdMap[rnti] = ueId;
    }

    // --------------------------------------------------------
    inline void UeScheduler::resetUeTerminal(UInt16 rnti, UInt8 ueId) {
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

        it = m_harqIdUeIdMap.begin();
        while (it != m_harqIdUeIdMap.end()) {
            if (it->second == ueId) {
                m_harqIdUeIdMap.erase(it++);
            } else {
                it++;
            }
        }
    }
}

#endif
