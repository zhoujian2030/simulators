/*
 * UeNotSendMsg3.h
 *
 *  Created on: Sep 29, 2018
 *      Author: zhoujian
 */

#ifndef UENOTSENDMSG3_H_
#define UENOTSENDMSG3_H_

#include "UeTerminal.h"

namespace ue {

	class PhyMacAPI;
	class StsCounter;

	class UeNotSendMsg3 : public UeTerminal {
	public:
		UeNotSendMsg3(UInt8 ueId, UInt16 raRnti, UInt16 preamble, PhyMacAPI* phyMacAPI, StsCounter* stsCounter);
		virtual ~UeNotSendMsg3();

		virtual void resetChild();

	protected:


	};

}


#endif /* UENOTSENDMSG3_H_ */
