/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include <map>

namespace sim_mob
{

class Role;
class TripChainItem;


/**
 * Class which handles the creation of Roles.
 *
 * \author Seth N. Hetu
 *
 * This class allows one to create Roles without the knowledge of where those Roles are
 * implemented. This is particularly useful when two roles have the same identity (short + medium) but
 * very different functionality.
 *
 * In addition, it does the following:
 *   \li Allows for future language-independence (e.g., a Role written in Python/Java)
 *   \li Allows the association of Roles with strings (needed by the config file).
 *   \li Allows full delayed loading of Roles, by saving the actual config string.
 */
class RoleFactory {
public:
	///Is this a Role that our Factory knows how to construct?
	bool isKnownRole(const std::string& roleName) const;

	///Retrieve the name of this Role's mode as it is known to the TripChain class.
	std::string getTripChainMode(const std::string& roleName) const;

	///Return a map of required attributes, with a flag on each set to false.
	std::map<std::string, bool> getRequiredAttributes(const std::string& roleName) const;

	///Create a role given its name and property list
	sim_mob::Role* createRole(const std::string& name, const std::map<std::string, std::string>& props) const;

	///Create a Role based on the current TripChain item.
	sim_mob::Role* createRole(const sim_mob::TripChainItem* const currTripChainItem) const;

private:


};


}
