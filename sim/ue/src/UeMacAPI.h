/*
 * UeMacAPI.h
 *
 *  Created on: Nov 04, 2016
 *      Author: j.zhou
 */

#ifndef UE_MAC_API
#define UE_MAC_API

#include "UdpSocketListener.h"
#include "UdpSocket.h"
#include "FapiInterface.h"
#include <string.h>

#define L2_SERVER_IP "192.168.64.140"
#define L2_SERVER_PORT 8888

#define UE_SERVER_IP "192.168.64.137"
#define UE_SERVER_PORT 9999

namespace ue {

    class UeScheduler;

    class UeMacAPI : public net::UdpSocketListener {
    public:
        UeMacAPI(UInt8* theBuffer);
        virtual ~UeMacAPI();

        virtual void handleRecvResult(net::UdpSocket* theSocket, int numOfBytesRecved);
        virtual void handleSendResult(net::UdpSocket* theSocket, int numOfBytesSent);
        virtual void handleCloseResult(net::UdpSocket* theSocket);
        virtual void handleErrorResult(net::UdpSocket* theSocket);

        void sendData();

    private:
        void handleSubFrameInd(UInt16 sfnsf);

        friend class UeTerminal;

        void resetSendBuffer();
        UInt8* getRachBuffer();
        UInt8* getSchBuffer();
        UInt8* getCrcBuffer();
        UInt8* getHarqBuffer();        
        UInt8* getSrBuffer();
        void addRachDataLength(UInt16 length);
        void addSchDataLength(UInt16 length);
        void addCrcDataLength(UInt16 length);
        void addHarqDataLength(UInt16 length);
        void addSrDataLength(UInt16 length);

        void addSchPduData(UInt8* theBuffer, UInt32 length);

        void logBuff(UInt8* theBuffer, UInt32 length);

        UInt8* m_recvBuff;
        UInt32 m_globalTick;
        UInt8 m_sf;
        UInt16 m_sfn;

        UeScheduler* m_ueScheduler;

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

        net::Socket::InetAddressPort m_l2Address;
        net::UdpSocket* m_txL2Socket;
    };

    // ------------------------------------
    inline UInt8* UeMacAPI::getRachBuffer() {
        return m_rachBuffer;
    }

    // ------------------------------------
    inline UInt8* UeMacAPI::getSchBuffer() {
        return m_schBuffer;
    }

    // ------------------------------------
    inline UInt8* UeMacAPI::getCrcBuffer() {
        return m_crcBuffer;
    }

    // ------------------------------------
    inline UInt8* UeMacAPI::getHarqBuffer() {
        return m_harqBuffer;
    }

     // ------------------------------------
    inline UInt8* UeMacAPI::getSrBuffer() {
        return m_srBuffer;
    }

    // ------------------------------------
    inline void UeMacAPI::addRachDataLength(UInt16 length) {
        m_rachDataLength += length;
    }

    // ------------------------------------
    inline void UeMacAPI::addSchDataLength(UInt16 length) {
       m_schDataLength += length;
    }

    // ------------------------------------
    inline void UeMacAPI::addCrcDataLength(UInt16 length) {
        m_crcDataLength += length;
    }

    // ------------------------------------
    inline void UeMacAPI::addHarqDataLength(UInt16 length) {
        m_harqDataLength += length;
    }

    // ------------------------------------
    inline void UeMacAPI::addSrDataLength(UInt16 length) {
        m_srDataLength += length;
    }

    // --------------------------------------
    inline void UeMacAPI::addSchPduData(UInt8* theBuffer, UInt32 length) {
        memcpy((void*)(m_schPduBuffer + m_schPduLength), (void*)theBuffer, length);
        m_schPduLength += length;
    }

}

#endif