/*
 * RrcLayer.h
 *
 *  Created on: Nov 25, 2016
 *      Author: j.zhou
 */
 
#ifndef RRC_LAYER_H
#define RRC_LAYER_H

#include "UeBaseType.h"

namespace ue {

    typedef enum {
        INVALID_MSG,
        IDENTITY_REQUEST,
        ATTACH_REJECT,
        RRC_RELEASE
    } RRC_NAS_MSG_TYPE;

    typedef enum {
        ID_TYPE_IMSI = 1,        
        ID_TYPE_IMEI = 2,
        ID_TYPE_IMEISV = 3,
        ID_TYPE_TMSI = 4
    } IDENTITY_TYPE;

    typedef struct {
        UInt8 identityType;
    } IdentityRequest;

    typedef struct {
        UInt8 spare;
    } AttachReject;

    typedef struct {
        UInt8 cause;
    } RrcRelease;

    typedef union {
        IdentityRequest identityReq;
        AttachReject attachReject;
        RrcRelease rrcRelease;
    } RrcMsg;

    class UeTerminal;

    class RrcLayer {
    public:
        RrcLayer(UeTerminal* ue);
        ~RrcLayer();

        void handleRxRRCMessage(UInt8* buffer, UInt32 length);

        void buildIdentityResponse(UInt8* buffer, UInt32& length);

    private:
        UInt32 decode(UInt8* buffer, UInt32 length, RrcMsg* rrcMsg);

        UeTerminal* m_ueTerminal;
    };
}

#endif
