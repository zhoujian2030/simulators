/*
 * TcpServerInterface.h
 *
 *  Created on: June 10, 2016
 *      Author: z.j
 */

#ifndef TCP_SERVER_INTERFACE_H
#define TCP_SERVER_INTERFACE_H

#include "TcpData.h"

namespace net {

    class TcpServerInterface {
    public:
        virtual void sendData(TcpData* theTcpData) = 0;
    };

}

#endif
