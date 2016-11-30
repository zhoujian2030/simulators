/*
 * PosixMQ.h
 *
 *  Created on: Mar 23, 2016
 *      Author: z.j
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string>

namespace mq {
    class PosixMQ {
    public:
        PosixMQ();
        ~PosixMQ();

        bool send(std::string msg);
        bool recv();

    private:
        // MQ ID
        mqd_t m_mqID;

    };
}