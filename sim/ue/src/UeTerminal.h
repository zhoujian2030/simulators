/*
 * UeTerminal.h
 *
 *  Created on: Nov 05, 2016
 *      Author: j.zhou
 */

#ifndef UE_TERMINAL_H
#define UE_TERMINAL_H

#include "FapiInterface.h"
#include "RrcLayer.h"

#define FAPI_HEADER_LENGTH 4

namespace ue {

    class PhyMacAPI;
    class UeScheduler;
    class HarqEntity;
    class RlcLayer;
    class PdcpLayer;
    class RrcLayer;
    class StsCounter;

    class UeTerminal {
    public:
        UeTerminal(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
        virtual ~UeTerminal();

        void updateConfig(UInt32 maxAccessCount);
        void showConfig();

        void reset();
        virtual void resetChild();

        BOOL schedule(UInt16 sfn, UInt8 sf, UeScheduler* pUeScheduler);

        void handleCreateUeReq(UInt16 srConfigIndex);
        void handleDeleteUeReq();
        void handleDlDciPdu(FAPI_dlConfigRequest_st* pDlConfigHeader, FAPI_dciDLPduInfo_st* pDlDciPdu);
        void handleDlSchPdu(FAPI_dlConfigRequest_st* pDlConfigHeader, FAPI_dlSCHConfigPDUInfo_st* pDlSchPdu);
        void handleDlTxData(FAPI_dlDataTxRequest_st* pDlDataTxHeader, FAPI_dlTLVInfo_st *pDlTlv, UeScheduler* pUeScheduler);
        void displayDciPduInfo(FAPI_dciDLPduInfo_st* pDlDciPdu); 

        void handleUlSchPdu(FAPI_ulConfigRequest_st* pUlConfigHeader, FAPI_ulSCHPduInfo_st* pUlSchPdu);
        void displayUlSchPduInfo(FAPI_ulSCHPduInfo_st* pUlSchPdu);

        void handleDci0Pdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlDCIPduInfo_st* pDci0Pdu);
        BOOL handleHIPdu(FAPI_dlHiDCIPduInfo_st* pHIDci0Header, FAPI_dlHiPduInfo_st* pHiPdu);

        // ------------------------------------------
        // callback functions of dl harq process 
        virtual void allocateDlHarqCallback(UInt16 harqProcessNum, BOOL result);
        void dlHarqSchConfigCallback(UInt16 harqProcessNum, BOOL result);
        void dlHarqReceiveCallback(UInt16 harqProcessNum, UInt8* theBuffer, UInt32 byteLen, BOOL result);
        virtual void dlHarqResultCallback(UInt16 harqProcessNum, UInt8 ackFlag, BOOL firstAck, BOOL result);

        // ------------------------------------------
        // callback functions of ul harq process
        void allocateUlHarqCallback(UInt16 harqId, BOOL result);
        void ulHarqSendCallback(UInt16 harqId, UInt8 numRb, UInt8 mcs, UInt8& ueState);
        // result TRUE: ack, FALSE: nack
        virtual void ulHarqResultCallback(UInt16 harqId, BOOL result, UInt8 ueState);
        void ulHarqTimeoutCallback(UInt16 harqId, UInt8 ueState);

        void handleDlConfigReq(FAPI_dlConfigRequest_st* pDlConfigReq);
        void handleUlConfigReq(FAPI_ulConfigRequest_st* pUlConfigReq);

        typedef enum {
            IDLE = 0,

            MSG1_SENT,
            MSG2_DCI_RECVD,
            MSG2_SCH_RECVD,
            MSG2_RECVD,

            MSG3_SENT,

            MSG4_DCI_RECVD,
            MSG4_SCH_RECVD,
            MSG4_RECVD,
            MSG4_ACK_SENT,

            RRC_SETUP_DCI_RECVD,   //10
            RRC_SETUP_SCH_RECVD,
            RRC_SETUP_RECVD,
            RRC_SETUP_ACK_SENT,
            RRC_SETUP_RETRANSMIT,

            RRC_REJ_RECVD,

            RRC_SETUP_COMPLETE_SR_SENT,

            RRC_SETUP_COMPLETE_SR_DCI0_RECVD,
            RRC_SETUP_COMPLETE_BSR_SENT,
            RRC_SETUP_COMPLETE_BSR_ACK_RECVD,

            RRC_SETUP_COMPLETE_DCI0_RECVD,  
            RRC_SETUP_COMPLETE_SENT,
            RRC_SETUP_COMPLETE_ACK_RECVD,   //22
            RRC_SETUP_COMPLETE_NACK_RECVD,

            RRC_IDENTITY_REQ_RECVD,

            RRC_CONNECTED, //25
            RRC_RELEASING,
            WAIT_TERMINATING
        } E_UE_STATE;

        typedef enum {
            SUB_ST_IDLE,
            SUB_ST_DCI0_RECVD,
            SUB_ST_TB_SENT
        } E_UE_SUB_STATE;

        enum {
            SUBFRAME_SENT_RACH = 3,//,
            MIN_RAR_LENGTH = 7,
            CONTENTION_RESOLUTION_LENGTH = 7,
            MSG3_LENGTH = 22,
            RRC_SETUP_COMPLETE_LENGTH = 85,  //TBD, depends on numRb allocated, 4 RB, mcs = 10
            BSR_MSG_LENGTH = 18, //TBD, 1 RB, mcs = 10

            NUM_UL_HARQ_PROCESS = 2,    // for TDD UL/DL CONFIG 2
            NUM_DL_HARQ_PROCESS = 10,

            IDENTITY_MSG_LENGTH = 41  // 2RB, mcs = 10
        };

        enum {
            subframeDelayAfterRachSent = 3,
            raResponseWindowSize = 7,   // from SIB2
            macContentionResolutionTimer = 48, // from SIB2
            identityRequestTimer = 1000,  // self-defined
            tddAckNackFeedbackMode = 0,  // bundling mode, from RRC setup
            periodicBSRTimer = 20, // from RRC setup
            reTxBsrTimer = 320,   // from RRC setup
            msg4AckRespTimer = 1800   // timer to wait response to RRC Request from RRC, must be less than T300
        };

        struct RarUlGrant {
            UInt8 hoppingFlag;
            UInt16 rbMap;
            UInt8 coding;
            UInt8 tpc;
            UInt8 ulDelay;
            UInt8 cqiReq;
        };

        struct RandomAccessResp {
            UInt8 rapid;
            UInt16 ta;
            // UInt32 ulGrant;
            RarUlGrant ulGrant;
            UInt16 tcRnti;
        };

        union DlSchMsg {
            RandomAccessResp rar;
        };

        enum RRCTimer {
            // refer to SIB2, 36.331 7.2, 7.3
            T300 = 2000,  // 2000ms
            T301 = 2000,
            T310 = 2000,
            N310 = 20,
            T311 = 1000,
            N311 = 1
        };

        enum E_UeEstabCause {
            emergency = 0,
            highPrio = 1,
            mt = 2,
            mo = 3,
            moData = 4
        };
        enum E_LcId {
            lc_ccch = 0x00,
            lc_contention_resolution = 0x1c,
            lc_short_bsr = 0x1d,
            lc_long_bsr = 0x1e,
            lc_padding = 0x1f
        };

        typedef enum {
            LCID1 = 1,
            LCID2 = 2,
            LCID3 = 3,
            LCID4 = 4,
            LCID5 = 5,
            LCID6 = 6,
            LCID7 = 7,
            LCID8 = 8,
            LCID9 = 9,
            LCID10 = 10,
            TA_CMD = 0x1d,
            PADDING = 0x1f
        } E_DL_SCH_LCID;

        struct LcIdItem {
            UInt8 lcId;
            UInt32 length;
            UInt8* buffer;
        };

        struct Msg3 {
            UInt8 lcCCCH;
            UInt8 lcPadding;
            UInt32 randomValue;
            UInt8 cause;
        };

    protected:
        BOOL scheduleRach(UeScheduler* pUeScheduler);
        void scheduleMsg3(UeScheduler* pUeScheduler);
        void scheduleSR(UeScheduler* pUeScheduler);
        void scheduleDCCH(UeScheduler* pUeScheduler);
        void processDlHarq(UeScheduler* pUeScheduler);
        void processTimer(UeScheduler* pUeScheduler);

        BOOL m_suspend;

        UInt32 m_accessCount;
        UInt32 m_maxAccessCount;

        // timer to wait RRC connection setup complete
        SInt32 m_t300Value;
        void startT300();
        void stopT300();
        BOOL isT300Expired();

        BOOL validateDlSfnSf(UInt16 sfn, UInt8 sf);

        BOOL parseRarPdu(UInt8* pduData, UInt32 pduLen);
        BOOL validateRar(RandomAccessResp* rar);
        void setSfnSfForMsg3();

        // timer to wait contention resolution after MSG3 sent
        SInt16 m_contResolutionTValue;
        void startContentionResolutionTimer();
        void stopContentionResolutionTimer();
        BOOL processContentionResolutionTimer();

        // timer to wait RRC Setup after MSG4 HARQ ACK sent
        SInt16 m_rrcSetupTValue;
        void startRRCSetupTimer();
        void stopRRCSetupTimer();
        BOOL processRRCSetupTimer();

        virtual void buildMsg3Data();
        void buildMsg3WithRnti();
        void buildCrcData(UInt8 crcFlag);
        void buildRRCSetupComplete();
        void buildBSRAndData(BOOL isLongBSR = FALSE);

        virtual BOOL parseContentionResolutionPdu(UInt8* data, UInt32 pduLen);

        UInt16 parseMacCCCHPdu(UInt8* data, UInt32 pduLen);
        virtual void handleCCCHMsg(UInt16 rrcMsgType);
        void setSfnSfForSR(BOOL isRetransmitSR = FALSE);
        void requestUlResource();

        friend class HarqEntity;

        HarqEntity* m_harqEntity;

        // timer to wait DCI0 after SR sent
        SInt16 m_srTValue;
        void startSRTimer();
        void stopSRTimer();
        BOOL isSRSent();
        BOOL processSRTimer();

        SInt16 m_bsrTValue;
        void startBSRTimer();
        void stopBSRTimer();
        BOOL isNonZeroBSRSent();
        BOOL processBSRTimer();

        void parseMacPdu(UInt8* data, UInt32 pduLen);

        friend class RrcLayer;
        friend class RlcLayer;
        RlcLayer* m_rlcLayer; 
        RrcLayer* m_rrcLayer;
        PdcpLayer* m_pdcpLayer;
        virtual void rrcCallback(UInt32 msgType, RrcMsg* msg);
        virtual void rlcCallback(UInt8* statusPdu, UInt32 length);

        UInt16 getUlTBSize(UInt8 numRb, UInt8 mcs);

        BOOL m_triggerIdRsp;
        BOOL m_triggerRlcStatusPdu;
        UInt8 m_rlcStatusPdu[2];

        static const UInt8 m_ulSubframeList[10];

        PhyMacAPI* m_phyMacAPI;
        DlSchMsg* m_dlSchMsg;
        UInt8 m_ueId;

        StsCounter* m_stsCounter;

        // for different UE, <m_raRnti, m_preamble> must be unique
        // m_raRnti could be the same with different preamble, not SUPPORTED yet 
        UInt16 m_raRnti;
        UInt16 m_preamble;
        UInt16 m_ta;
        UInt16 m_rachTa;
        
        UInt32 m_state;
        UInt32 m_subState;

        // subframe in which to send rach
        UInt8 m_rachSf;
        UInt16 m_rachSfn;
        UInt16 m_raTicks;

        UInt16 m_rachSfnDelay;
        BOOL m_firstRachSfnSet;
        BOOL m_firstRachSent;

        UInt32 m_maxRachIntervalSfn;
        UInt32 m_rachIntervalSfnCnt;

        // subframe in which to send MSG3
        // this can be calculated according the sfnsf received Rar 
        // MAC will send UL config to phy one tick earlier for 
        // receiving the MSG3, e.g. UE is expected to send MSG3 in 
        // tick 1.2, MAC sends UL config to PHY in tick 1.1 
        // MAC will free temp rnti resource if not receive MSG3 before
        // tick 1.6 without retransmitting RAR.
        UInt8 m_msg3Sf;
        UInt16 m_msg3Sfn;
        Msg3 m_msg3;

        UInt16 m_sfn;
        UInt8 m_sf;

        // UInt16 m_provSfnSf;
        UInt16 m_provSfn;
        UInt8 m_provSf;

        UInt16 m_rnti;

        // subframe in which to send SR 
        BOOL m_needSendSR;
        UInt8 m_srConfigIndex;
        SInt8 m_srPeriodicity;
        UInt8 m_dsrTransMax;
        UInt8 m_srCounter;
        UInt16 m_srSfn;
        UInt8 m_srSf;

        Char8 m_uniqueId[30];

        BOOL m_biRecvd;
    };

    // ------------------------------------------------------
    inline BOOL UeTerminal::validateDlSfnSf(UInt16 sfn, UInt8 sf) {
        if (sfn == m_sfn) {
            return (m_sf <= sf);
        }

        return (((m_sfn + 1)%1024) == sfn);
    }

    // ------------------------------------------------------
    inline BOOL UeTerminal::validateRar(RandomAccessResp* rar) {
        return (rar->rapid == m_preamble) && (rar->ta == m_rachTa);
    }

    // ------------------------------------------------------
    inline void UeTerminal::setSfnSfForMsg3() {
        // refer to 36.213, section 6.1.1 
        UInt8 minK1 = 6;
        UInt8 sf = m_provSf + minK1;
        UInt16 sfn = m_provSfn;
        BOOL foundFirstUlSf = FALSE;
        while (1) {
            if (sf >= 10) {
                sf = sf - 10;
                sfn = (sfn + 1) % 1024;
            }

            if (m_ulSubframeList[sf] == 1) {
                if (m_dlSchMsg->rar.ulGrant.ulDelay == 0) {
                    // if ulDelay = 0
                    // send MSG3 in the first UL subframe starting from n+k1 subframe,
                    // where n is the subframe received MSG2, k1 >= 6
                    m_msg3Sf = sf;
                    m_msg3Sfn = sfn;
                    break;
                } else {
                    // if ulDelay = 1
                    // send MSG3 in the second UL subframe starting from n+k1 subframe,
                    if (foundFirstUlSf) {
                        m_msg3Sf = sf;
                        m_msg3Sfn = sfn;
                        break;
                    } else {                        
                        foundFirstUlSf = TRUE;
                    }
                }
            }

            ++sf;
        }
    }

    // --------------------------------------------------------
    inline void UeTerminal::startContentionResolutionTimer() {
        m_contResolutionTValue = macContentionResolutionTimer;
    }

    // --------------------------------------------------------
    inline void UeTerminal::stopContentionResolutionTimer() {
        m_contResolutionTValue = -1;
    }

    // --------------------------------------------------------
    inline void UeTerminal::startRRCSetupTimer() {
        m_rrcSetupTValue = msg4AckRespTimer;
    }

    // --------------------------------------------------------
    inline void UeTerminal::stopRRCSetupTimer() {
        m_rrcSetupTValue = -1;
    }

    // -------------------------------------------------------
    inline void UeTerminal::startT300() {
        m_t300Value = T300;
    }

    // -------------------------------------------------------
    inline void UeTerminal::stopT300() {
        m_t300Value = -1;
    } 

    // -------------------------------------------------------
    inline BOOL UeTerminal::isT300Expired() {
        if (m_t300Value < 0) {
            return FALSE;
        }
        return ((--m_t300Value) == 0);
    } 

    // -------------------------------------------------------
    inline void UeTerminal::startSRTimer() {
        m_srTValue = m_srPeriodicity;
    }

    // -------------------------------------------------------
    inline void UeTerminal::stopSRTimer() {
        m_srTValue = -1;
    }

    
    // -------------------------------------------------------
    inline BOOL UeTerminal::isSRSent() {
        return ((m_srTValue > 0) && (m_srTValue <= m_srPeriodicity));
    }

    // -------------------------------------------------------
    inline void UeTerminal::startBSRTimer() {
        m_bsrTValue = reTxBsrTimer;
    }

    // -------------------------------------------------------
    inline void UeTerminal::stopBSRTimer() {
        m_bsrTValue = -1;
    }

    // -------------------------------------------------------
    inline BOOL UeTerminal::isNonZeroBSRSent() {
        return ((m_bsrTValue > 0) && (m_bsrTValue <= reTxBsrTimer));
    }
}

#endif
