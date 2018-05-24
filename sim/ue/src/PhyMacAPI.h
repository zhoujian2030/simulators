/*
 * PhyMacAPI.h
 *
 *  Created on: Nov 04, 2016
 *      Author: j.zhou
 */

#ifndef UE_MAC_API
#define UE_MAC_API

#ifdef OS_LINUX
#include "UdpSocketListener.h"
#include "UdpSocket.h"
#endif

#include "FapiInterface.h"
#include <string.h>

#ifdef OS_LINUX
#define L2_SERVER_IP "127.0.0.1"
#define L2_SERVER_PORT 8888

#define UE_SERVER_IP "127.0.0.1"
#define UE_SERVER_PORT 9999
#endif

namespace ue {

    class UeScheduler;
    class StsCounter;

#ifdef OS_LINUX
    class PhyMacAPI : public net::UdpSocketListener {
#else
	class PhyMacAPI {
#endif
    public:
        PhyMacAPI(UInt8* theBuffer);
        virtual ~PhyMacAPI();

#ifdef OS_LINUX
        virtual void handleRecvResult(net::UdpSocket* theSocket, int numOfBytesRecved);
        virtual void handleSendResult(net::UdpSocket* theSocket, int numOfBytesSent);
        virtual void handleCloseResult(net::UdpSocket* theSocket);
        virtual void handleErrorResult(net::UdpSocket* theSocket);
#else
        void handleDlDataRequest(UInt8* theBuffer, SInt32 length);
#endif
        void handleSubFrameInd(UInt16 sfnsf);
        void sendData();
        void sendUeConfigResp();

        void updateUeConfig(UInt32 numUe, UInt32 numAccessCount);

    private:

        friend class UeTerminal;
        friend class NWRetransmitRrcSetup;
        friend class UENotSendRrcSetupComplete;
        friend class NWRetransmitIdentityReq;
        friend class UESuspending;
        friend class UENotSendRlcAck;
        friend class UESendRrcReestablishmentReq;
        friend class UeNotSendRlcAndHarqAck;
        friend class UENotSendHarqAckForMsg4;

        void resetSendBuffer();
        UInt8* getRachBuffer();
        UInt8* getSchBuffer();
        UInt8* getCrcBuffer();
        UInt8* getHarqBuffer();        
        UInt8* getSrBuffer();
        UInt8* getUeConfigMsgBuffer();
        void addRachDataLength(UInt16 length);
        void addSchDataLength(UInt16 length);
        void addCrcDataLength(UInt16 length);
        void addHarqDataLength(UInt16 length);
        void addSrDataLength(UInt16 length);
        void addUeConfigMsgLength(UInt16 length);
        UInt16 getSchDataLength();

        void addSchPduData(UInt8* theBuffer, UInt32 length);

        void logBuff(UInt8* theBuffer, UInt32 length);

        UInt8* m_recvBuff;
        UInt32 m_globalTick;
        UInt8 m_sf;
        UInt16 m_sfn;

        BOOL m_isRunning;

        UeScheduler* m_ueScheduler;
        StsCounter* m_stsCounter;

        UInt16 m_rachDataLength;
        UInt8 m_rachBuffer[SOCKET_BUFFER_LENGTH];

        UInt16 m_schDataLength;
        UInt8 m_schBuffer[SOCKET_BUFFER_LENGTH];

        UInt16 m_crcDataLength;
        UInt8 m_crcBuffer[SOCKET_BUFFER_LENGTH];

        UInt16 m_harqDataLength;
        UInt8 m_harqBuffer[SOCKET_BUFFER_LENGTH];

        UInt16 m_srDataLength;
        UInt8 m_srBuffer[SOCKET_BUFFER_LENGTH];

        UInt16 m_schPduLength;
        UInt8 m_schPduBuffer[SOCKET_BUFFER_LENGTH];

        UInt16 m_ueConfigMsgLength;
        UInt8 m_ueConfigMsgBuffer[SOCKET_BUFFER_LENGTH];

#ifdef OS_LINUX
        net::Socket::InetAddressPort m_l2Address;
        net::UdpSocket* m_txL2Socket;
#endif
    };

    // ------------------------------------
    inline UInt8* PhyMacAPI::getRachBuffer() {
        return m_rachBuffer;
    }

    // ------------------------------------
    inline UInt8* PhyMacAPI::getSchBuffer() {
        return m_schBuffer;
    }

    // ------------------------------------
    inline UInt8* PhyMacAPI::getCrcBuffer() {
        return m_crcBuffer;
    }

    // ------------------------------------
    inline UInt8* PhyMacAPI::getHarqBuffer() {
        return m_harqBuffer;
    }

     // ------------------------------------
    inline UInt8* PhyMacAPI::getSrBuffer() {
        return m_srBuffer;
    }

	// ------------------------------------
	inline UInt8* PhyMacAPI::getUeConfigMsgBuffer() {
	   return m_ueConfigMsgBuffer;
	}

    // ------------------------------------
    inline void PhyMacAPI::addRachDataLength(UInt16 length) {
        m_rachDataLength += length;
    }

    // ------------------------------------
    inline void PhyMacAPI::addSchDataLength(UInt16 length) {
       m_schDataLength += length;
    }

    // ------------------------------------
    inline UInt16 PhyMacAPI::getSchDataLength() {
    	return m_schDataLength;
    }

    // ------------------------------------
    inline void PhyMacAPI::addCrcDataLength(UInt16 length) {
        m_crcDataLength += length;
    }

    // ------------------------------------
    inline void PhyMacAPI::addHarqDataLength(UInt16 length) {
        m_harqDataLength += length;
    }

    // ------------------------------------
    inline void PhyMacAPI::addSrDataLength(UInt16 length) {
        m_srDataLength += length;
    }

    // ------------------------------------
    inline void PhyMacAPI::addUeConfigMsgLength(UInt16 length) {
    	m_ueConfigMsgLength += length;
    }

    // --------------------------------------
    inline void PhyMacAPI::addSchPduData(UInt8* theBuffer, UInt32 length) {
        memcpy((void*)(m_schPduBuffer + m_schPduLength), (void*)theBuffer, length);
        m_schPduLength += length;
    }

}

#endif
