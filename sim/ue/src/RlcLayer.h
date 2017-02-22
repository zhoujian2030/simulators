/*
 * RlcLayer.h
 *
 *  Created on: Nov 24, 2016
 *      Author: j.zhou
 */
 
#ifndef RLC_LAYER_H
#define RLC_LAYER_H

#include "UeBaseType.h"
#include "CLogger.h"

namespace ue {

    class PdcpLayer;
    class UeTerminal;

    class RlcLayer {
    public:
        RlcLayer(UeTerminal* ue, PdcpLayer* pdcpLayer);
        ~RlcLayer();

        typedef struct {
            UInt8  dc;
            UInt8  rf;
            UInt8  p;
            UInt8  fi;
            UInt8  e;
            UInt8  lsf;
            UInt16  sn;
            UInt16 so;
            UInt16 soEnd;
        } AmdHeader;

        typedef struct {
            UInt8 tPollRetransmit;
            UInt8 pollPDU;
            UInt8 pollByte;
            UInt8 maxRetxThreshold;
            UInt8 tReordering;
            UInt8 tStatusProhibit;
        } RrcSetupConfig;       

        enum E_FI {
            FI_00_SINGLE_SEG,
            FI_01_FIRST_SEG,
            FI_02_LAST_SEG,
            FI_03_INTER_SEG
        };
        
        enum {
            MAX_RLC_AMD_SDU_SEG_LENGTH = 1024,
            MAC_RLC_AMD_SDU_SEG_NUM = 1024
        };

        struct Node {
            UInt8 buffer[MAX_RLC_AMD_SDU_SEG_LENGTH];
            UInt16 length;
            UInt8 sn;
            UInt8 e;

            Node* next;
            Node* prev;
        };

        class NodePool {
        public:
            NodePool(UInt32 size) {
                m_head = new Node();

                if (size <= 1) {
                    m_head->next = m_head;
                    m_head->prev = m_head;
                    return;
                }

                Node* curNode = m_head;
                for (UInt32 i=0; i<size-1; i++) {
                    Node* node = new Node();
                    node->next = m_head;
                    node->prev = curNode;
                    curNode->next = node;
                    curNode = node;
                }
                m_head->prev = curNode;
            }

            ~NodePool() {
                Node* curNode = m_head->prev;
                while(curNode != m_head) {
                    curNode->prev->next = m_head;
                    m_head->prev = curNode->prev;
                    delete curNode;
                    curNode = m_head->prev;
                }

                delete m_head;
            }

            Node* getNode() {
                Node* node = 0;
                if (m_head != 0) {
                    node = m_head->prev;
                    if (node == m_head) {
                        // the pool is empty after allocating this node
                        LOG_WARN(UE_LOGGER_NAME, "[%s], get last node!\n", __func__);
                        m_head = 0;
                    } else {
                        node->prev->next = m_head;
                        m_head->prev = node->prev;
                    }
                }

                LOG_DBG(UE_LOGGER_NAME, "[%s], get node = %p\n", __func__, node);

                return node;
            }

            void freeNode(Node* node) {
                if (node != 0) {
                    LOG_DBG(UE_LOGGER_NAME, "[%s], free node = %p\n", __func__, node);
                    if (m_head == 0) {
                        m_head = node;
                        m_head->prev = node;
                        m_head->next = node;
                    } else {
                        m_head->prev->next = node;
                        node->prev = m_head->prev;
                        m_head->prev = node;
                        node->next = m_head;
                    }
                }
            }

        private:
            Node* m_head;
        };

        void reset();

        void handleRxAMDPdu(UInt8* buffer, UInt32 length);
        void buildRlcAMDHeader(UInt8* buffer, UInt32& length);

    private:

        void reassembleAMDPdu(AmdHeader* header, UInt8* buffer, UInt32 length);
        void handleExtField(UInt32& sduLength, UInt8* buffer, UInt32 length);
        void freeAllSegments();

        RrcSetupConfig m_rrcSetupConfig;

        UeTerminal* m_ue;
        PdcpLayer* m_pdcpLayer;
        AmdHeader m_amdHeader;

        NodePool* m_nodePool;
        Node* m_firstSeg;
        UInt16 m_numSegRecvd;

        UInt8 m_rlcSdu[1024*10];

        UInt16 m_sn;
    };

    inline void RlcLayer::freeAllSegments() {
        if (m_firstSeg == 0) {
            return;
        }

        while (1) {
            if (m_firstSeg->next == m_firstSeg) {
                m_nodePool->freeNode(m_firstSeg);
                m_firstSeg = 0;
                break;
            }

            Node* node = m_firstSeg;
            m_firstSeg = node->next;
            m_firstSeg->prev = node->prev;
            node->prev->next = m_firstSeg;

            m_nodePool->freeNode(node);
        }
    }
}

#endif
