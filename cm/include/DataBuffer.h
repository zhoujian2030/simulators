/*
 * DataBuffer.h
 *
 *  Created on: Apr 08, 2016
 *      Author: z.j
 */

#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include <assert.h>
#include <string>

namespace cm {
     
    // allocate a buffer with default 2000 bytes size, the user 
    // can write data to this buffer after getting the buffer
    // pointer by getEndOfDataPointer(), but note that the user
    // is responsible to call increaseDataLength() to update the
    // buffer length after it wirtes the data to buffer, or the 
    // new data could be overwriten by other operations
    class DataBuffer {
    public:
        DataBuffer();
        DataBuffer(int bufferLength);
        DataBuffer(const DataBuffer& theReceiveBuffer);
        DataBuffer(std::string bufferStr);
        virtual ~DataBuffer();

        // @description - increase the m_endOfDataPointer to point
        //      to the end of valid data inserted in m_buffer
        // @param numberOfBytes - number of new inserted bytes
        // @return the valid data length in m_buffer
        int increaseDataLength(int numberOfBytes);
        
        // @description - get the buffer size
        // @return the buffer size, default size if INITIAL_BUFFER_SIZE=2000
        int getSize() const;
        
        // @description - get the data length in buffer
        // @return the length of valid data in m_buffer
        int getLength() const;
        
        // @description - get remaining buffer size
        // @return getSize() - getLength()
        int getRemainBufferSize() const;

        char* getEndOfDataPointer() const;

        char* getStartOfDataPointer() const;
        
        void reset();

        // Only for test
        std::string getData() const;

        enum {
          INITIAL_BUFFER_SIZE = 2000  
        };

    private:
        char* m_buffer;
        unsigned int m_bufferSize;

        char* m_endOfDataPointer;
    };

    // --------------------------------------------
    inline int DataBuffer::increaseDataLength(int numberOfBytes) {
        assert((m_endOfDataPointer + numberOfBytes) <= (m_buffer + m_bufferSize));

        m_endOfDataPointer += numberOfBytes;

        return getLength();
    }

    // --------------------------------------------
    inline int DataBuffer::getSize() const {
        return m_bufferSize;
    }

    // --------------------------------------------
    inline int DataBuffer::getLength() const {
        return (m_endOfDataPointer - m_buffer);
    }
    
    // --------------------------------------------
    inline int DataBuffer::getRemainBufferSize() const {
        return m_bufferSize - (m_endOfDataPointer - m_buffer);
    }

    // --------------------------------------------
    inline char* DataBuffer::getEndOfDataPointer() const {
        return m_endOfDataPointer;
    }

    // --------------------------------------------
    inline char* DataBuffer::getStartOfDataPointer() const {
        return m_buffer;
    }
    
    // --------------------------------------------
    inline void DataBuffer::reset() {
        m_endOfDataPointer = m_buffer;
    }

    // --------------------------------------------
    inline std::string DataBuffer::getData() const {
        std::string dataStr;
        dataStr.append(m_buffer, getLength());

        return dataStr;
    }
}

#endif
