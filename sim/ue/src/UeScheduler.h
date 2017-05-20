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
#include <vector>

#define MAX_UE_SUPPORTED 4//32
#define MAX_UE_ACCESS_COUNT 9999999
#define DL_MSG_CONTAINER_SIZE 4 * 4

namespace ue {

    class UeTerminal;
    class PhyMacAPI;
    class StsCounter;

    struct Node {
        void* buffer;
        Node* next;
        Node* tail;
    };

    class NodePool {
    public:
        NodePool(UInt32 size);
//        {
//            m_head = new Node();
//            if (size > 0) {
//                size--;
//            }
//
//            while (size--) {
//                Node* node = new Node();
//                node->next = m_head;
//                m_head = node;
//            }
//        }

        ~NodePool();
//        {
//
//        }

        Node* getNode();
//        {
//            Node* retNode = m_head;
//            if (m_head != 0) {
//                m_head = m_head->next;
//                retNode->next = 0;
//                retNode->tail = 0;
//            }
//            return retNode;
//        }

        void freeNode(Node* node);
//        {
//            if (node != 0) {
//                node->next = m_head;
//                m_head = node;
//            }
//        }

    private:
        Node* m_head;
    };

    class UeScheduler {
    public:
        UeScheduler(PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
        ~UeScheduler();

        void updateSfnSf(UInt16 sfn, UInt8 sf);

        void processDlData(UInt8* buffer, SInt32 length);
        void handleUeConfigReq(FAPI_phyUeConfigRequest_st* pUeConfigReq);
        void handleCreateUeReq(UInt16 ueIndex, UInt16 rnti, UInt16 srConfigIndex);
        void handleDeleteUeReq(UInt16 rnti);
        void handleDlConfigReq(FAPI_dlConfigRequest_st* pDlConfigReq);
        void handleUlConfigReq(FAPI_ulConfigRequest_st* pUlConfigReq);
        void handleHIDci0Req(FAPI_dlHiDCIPduInfo_st* pHIDci0Req);
        void handleDlDataReq(FAPI_dlDataTxRequest_st* pDlDataReq);

        BOOL schedule();
        void processData();

        // callback function by UeTerminal
        void updateRntiUeIdMap(UInt16 rnti, UInt8 ueId);
        void resetUeTerminal(UInt16 rnti, UInt8 ueId);

    private:

        BOOL validateSfnSf(BOOL isULCfg, UInt16 sfn, UInt8 sf);

        static const UInt16 m_maxRaRntiUeId = MAX_UE_SUPPORTED;
        UInt16 m_sfn;
        UInt8  m_sf;
        UInt16 m_prevSfn;
        UInt8 m_prevSf;
        UeTerminal** m_ueList;
        std::map<UInt16, UInt8> m_rntiUeIdMap;
        std::map<UInt16, UInt8> m_pduIndexUeIdMap;
        // std::map<UInt16, UInt8> m_harqIdUeIdMap;
        std::vector<UInt32> m_ueIdHarqIdVect;

//#ifndef OS_LINUX
        std::map<UInt16, UInt16> m_ueIndexRntiMap;
//#endif

        struct FapiL1MsgHead {
            UInt16 sfnsf;
        };

        struct DlMsgBuffer {
            UInt8 data[SOCKET_BUFFER_LENGTH];
            SInt32 length;
        };

//        struct Node {
//            void* buffer;
//            Node* next;
//            Node* tail;
//        };
//
//        class NodePool {
//        public:
//            NodePool(UInt32 size) {
//                m_head = new Node();
//                if (size > 0) {
//                    size--;
//                }
//
//                while (size--) {
//                    Node* node = new Node();
//                    node->next = m_head;
//                    m_head = node;
//                }
//            }
//            ~NodePool() {
//
//            }
//
//            Node* getNode() {
//                Node* retNode = m_head;
//                if (m_head != 0) {
//                    m_head = m_head->next;
//                    retNode->next = 0;
//                    retNode->tail = 0;
//                }
//                return retNode;
//            }
//
//            void freeNode(Node* node) {
//                if (node != 0) {
//                    node->next = m_head;
//                    m_head = node;
//                }
//            }
//
//        private:
//            Node* m_head;
//        };

        DlMsgBuffer m_dlMsgBufferContainer[DL_MSG_CONTAINER_SIZE]; // only support 1 or 2 tick advance
        NodePool* m_nodePool;
        Node* m_dlDataNodeHead;
        Node* m_dlConfigNodeHead;
        Node* m_ueConfigNodeHead;
        Node* m_hiDci0NodeHead;

        DlMsgBuffer m_ulCfgMsg;
    };

    inline void UeScheduler::updateRntiUeIdMap(UInt16 rnti, UInt8 ueId) {
        m_rntiUeIdMap[rnti] = ueId;
    }
}

#endif
