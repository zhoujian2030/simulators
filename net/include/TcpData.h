/*
 * TcpData.h
 *
 *  Created on: June 10, 2016
 *      Author: z.j
 */

#ifndef TCP_DATA_H
#define TCP_DATA_H

#include <string>

namespace net {
    
    class TcpData {
    public:
        TcpData(unsigned long globalConnId);
        virtual ~TcpData();

        virtual const char* getData(int& dataLength);

        unsigned long getGlobalConnId() const;

        int getLength() const;

    protected:
        std::string m_data;
        // global tcp connection id
        unsigned long m_globalConnId;
    };

    // -----------------------------------------
    inline unsigned long TcpData::getGlobalConnId() const {
        return m_globalConnId;
    }

    // -----------------------------------------
    inline int TcpData::getLength() const {
        return m_data.length();
    }
}

#endif
