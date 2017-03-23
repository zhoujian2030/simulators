/*
 * UeMain.cpp
 *
 *  Created on: Nov 01, 2016
 *      Author: j.zhou
 */

#ifdef OS_LINUX

#include "UeService.h"
#include "SfnSfManager.h"
#include "StsCounter.h"

using namespace ue;

int main(int argc, char* argv[]) {

    UeService* service = new UeService("UE Mock");
    // cm::Thread::sleep(2000);
    StsCounter::getInstance();
    SfnSfManager::getInstance()->registerService(service);
    SfnSfManager::getInstance()->start();
    service->wait();

    delete service;

    return 0;
}

#endif
