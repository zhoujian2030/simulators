/*
 * TcpServerCallback.h
 *
 *  Created on: June 6, 2016
 *      Author: z.j
 */
 
#ifndef TCP_SERVER_CALLBACK_H
#define TCP_SERVER_CALLBACK_H

namespace net {
    
    // deliveryResult() and dataIndication() must be called by the same worker thread for the same 
    // TCP connection, but may be called by different worker thread for different TCP connections
    class TcpServerCallback {
    public:

        virtual void deliveryResult(unsigned int globalConnId, bool status) = 0;
        
        // this method is called when data is received on a socket all data in the buffer must be 
        // copied out by the receiver as the buffer will be cleared in lower layer after return, 
        // and it will be overwriten by reactor once there is new data received on the socket 
        //
        // Arguments:
        //  globalConnId - the global connection id used to identify a unique TCP connection
        //  buffer - buffer pointer of the received data
        //  numOfBytes - data length in the buffer
        //
        // Return:
        //  void
        virtual void dataIndication(unsigned int globalConnId, char* buffer, int numOfBytes) = 0;
        
        virtual void closeIndication(unsigned int globalConnId) = 0;
    };
    
}

#endif