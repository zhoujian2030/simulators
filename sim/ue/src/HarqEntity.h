/*
 * HarqEntity.h
 *
 *  Created on: Nov 14, 2016
 *      Author: j.zhou
 */

#ifndef HARQ_ENTITY_H
#define HARQ_ENTITY_H

#include "UeBaseType.h"
#include "FapiInterface.h"
#include <map>

namespace ue {

    class UeTerminal;

    class HarqEntity {
    public:
        HarqEntity(UInt16 numOfUlHarqProcess = 2, UInt16 numOfDlHarqProcess = 10);
        ~HarqEntity();

        void reset();

        // ------------------------------------------------
        // API of UL HARQ
        BOOL allocateUlHarqProcess(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlDCIPduInfo_st* pDci0Pdu, UeTerminal* pUeTerminal);
        void send(UeTerminal* pUeTerminal);     
        void handleAckNack(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlHiPduInfo_st* pHiPdu, UeTerminal* pUeTerminal);  
        void calcAndProcessUlHarqTimer(UeTerminal* pUeTerminal);
        void handleUlSchConfig(UInt16 sfnsf, void* ulSchPdu, UeTerminal* pUeTerminal);

        // ------------------------------------------------
        // API of DL HARQ
        void allocateDlHarqProcess(UInt16 sfnsf, FAPI_dciDLPduInfo_st* pDlDciPdu, UeTerminal* pUeTerminal);
        void handleDlSchConfig(UInt16 sfnsf, FAPI_dlSCHConfigPDUInfo_st* pDlSchPdu, UeTerminal* pUeTerminal);
        void receive(UInt16 sfnsf, UInt8* theBuffer, UInt32 byteLen, UeTerminal* pUeTerminal);
        void sendAck(UInt16 sfn, UInt8 sf, UeTerminal* pUeTerminal);

        // -------------------------------------------------
        // DL HARQ Process
        class DlHarqProcess {
        public:
            DlHarqProcess(UInt16 index);
            ~DlHarqProcess();

            BOOL isFree() const;            
            void free();

            void prepareReceiving(UInt16 sfn, UInt8 sf, UInt8 ueState);
            void handleDlSchConfig(UInt16 sfnsf, UeTerminal* pUeTerminal);
            void receive(UInt16 sfnsf, UInt8* theBuffer, UInt32 byteLen, UeTerminal* pUeTerminal);
            BOOL sendAck(UInt16 sfn, UInt8 sf, BOOL isFirstAckSent, UeTerminal* pUeTerminal);

        private:
            enum DL_HARQ_PROCESS_STATE {
                IDLE,
                PREPARE_RECEIVING,
                TB_RECEIVED
            };

            UInt16 m_index;
            UInt8 m_state;
            UInt8 m_ueState;

            UInt16 m_recvSfn;
            UInt8 m_recvSf;

            UInt16 m_harqAckSfn;
            UInt8 m_harqAckSf;  
        };

        // -------------------------------------------------
        // UL HARQ Process
        class UlHarqProcess {
        public:
            UlHarqProcess(UInt16 index);
            ~UlHarqProcess();

            BOOL isFree() const;                       
            void free();

            void prepareSending(UInt16 sfn, UInt8 sf, UInt8 ueState);
            void send(UInt16 harqId, UeTerminal* pUeTerminal);
            BOOL handleAckNack(UInt16 harqId, UInt16 sfnsf, UInt8 hiValue, UeTerminal* pUeTerminal);
            BOOL calcAndProcessTimer(UInt16 harqId, UeTerminal* pUeTerminal);
            void handleUlSchConfig(UInt16 harqId, UInt16 sfnsf, UeTerminal* pUeTerminal);

        private:
            enum UL_HARQ_PROCESS_STATE {
                IDLE,
                PREPARE_SENDING,
                TB_SENT
            };

            enum HARQ_TIMER_VALUE {
                K_PHICH_FOR_TDD_UL_DL_CONFIG_2 = 6
            };

            friend class UeTerminal;

            UInt16 m_index;
            UInt8 m_state;
            UInt8 m_ueState;

            UInt16 m_sendSfn;
            UInt8 m_sendSf;
            
            SInt8 m_timerValue;
            void startTimer();
            void stopTimer();
        };
        

    private:
        UlHarqProcess** m_ulHarqProcessList;
        UInt16 m_numUlHarqProcess;
        std::map<UInt16, UInt16> m_harqIdUlIndexMap;

        DlHarqProcess** m_dlHarqProcessList;
        UInt16 m_numDlHarqProcess;
        // use sfnsf as harqId as DL DCI, DL SCH and DL TX DATA sent in 
        // the same sfnsf. what about rbCoding?
        std::map<UInt32, UInt16> m_harqIdDlIndexMap;
    };

    // -------------------------------------------
    inline BOOL HarqEntity::UlHarqProcess::isFree() const {
        return (m_state == IDLE);
    }

    // -------------------------------------------    
    inline void HarqEntity::UlHarqProcess::startTimer() {
        m_timerValue = K_PHICH_FOR_TDD_UL_DL_CONFIG_2;
    }

    // -------------------------------------------    
    inline void HarqEntity::UlHarqProcess::stopTimer() {
        m_timerValue = -1;
    }

    // -------------------------------------------
    inline BOOL HarqEntity::DlHarqProcess::isFree() const {
        return (m_state == IDLE);
    }
}

#endif
