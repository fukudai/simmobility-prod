#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::linkAndCrossing_t_pimpl::pre ()
{
}

sim_mob::LinkAndCrossing sim_mob::xml::linkAndCrossing_t_pimpl::post_linkAndCrossing_t ()
{
	return model;
}

void sim_mob::xml::linkAndCrossing_t_pimpl::ID (unsigned char value)
{
	model.id = value;
}

void sim_mob::xml::linkAndCrossing_t_pimpl::linkID (unsigned int value)
{
	model.link = Links_pimpl::LookupLink(value);
}

void sim_mob::xml::linkAndCrossing_t_pimpl::crossingID (unsigned int value)
{
	//std::cout << "crossingID: " <<value << std::endl;
}

void sim_mob::xml::linkAndCrossing_t_pimpl::angle (unsigned char value)
{
	//std::cout << "angle: " << static_cast<unsigned short>(value) << std::endl;
}


