/*
 * ANNOUNCE_Handler.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

//#include "entities/commsim/communicator/message/base/Handler.hpp"
//#include "entities/commsim/communicator/message/base/Message.hpp"
#include "ANNOUNCE_Message.hpp"
namespace sim_mob {
namespace roadrunner
{
//class MSG_ANNOUNCE;
class HDL_ANNOUNCE : public Handler {

public:
//	HDL_ANNOUNCE();
	void handle(msg_ptr message_,Broker*);
};
}/* namespace roadrunner */
} /* namespace sim_mob */