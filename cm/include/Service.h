/*
 * Service.h
 *
 *  Created on: June 7, 2016
 *      Author: z.j
 */

#ifndef SERVICE_H
#define SERVICE_H

#include "Thread.h"
#include <string>

namespace cm {

    // The base class for all services which are responsible
    // to intialize a standlone service(process), like a TCP
    // server, HTTP server, etc.
    // The inherrited class should implements method run() to
    // do the service initialization jobs.
    class Service : public Thread {
    public:
        Service(std::string serviceName);
        virtual ~Service();

        // @description - start the service thread as joinable
        void init();
    };

}

#endif
