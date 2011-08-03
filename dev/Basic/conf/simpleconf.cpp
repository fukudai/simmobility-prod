/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "simpleconf.hpp"

#include <tinyxml.h>

//Include here (forward-declared earlier) to avoid include-cycles.
#include "../entities/Agent.hpp"
#include "../entities/Person.hpp"
#include "../entities/Region.hpp"
#include "../entities/roles/Pedestrian.hpp"
#include "../entities/roles/Driver.hpp"
#include "../geospatial/aimsun/Loader.hpp"

using std::map;
using std::string;

using namespace sim_mob;


namespace {



int getValueInMS(int value, const std::string& units)
{
	//Detect errors
	if (units.empty() || (units!="minutes" && units!="seconds" && units!="ms")) {
		return -1;
	}

	//Reduce to ms
    if (units == "minutes") {
    	value *= 60*1000;
    }
    if (units == "seconds") {
    	value *= 1000;
    }

    return value;
}


int ReadGranularity(TiXmlHandle& handle, const std::string& granName)
{
	TiXmlElement* node = handle.FirstChild(granName).ToElement();
	if (!node) {
		return -1;
	}

	int value;
	const char* units = node->Attribute("units");
	if (!node->Attribute("value", &value) || !units) {
		return -1;
	}

	return getValueInMS(value, units);
}


bool loadXMLAgents(TiXmlDocument& document, std::vector<Agent*>& agents, const std::string& agentType)
{
	//Quick check.
	if (agentType!="pedestrian" && agentType!="driver") {
		return false;
	}

	//Build the expression dynamically.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(agentType+"s").FirstChild(agentType).ToElement();
	if (!node) {
		//Agents are optional
		return true;
	}

	//Loop through all agents of this type
	for (;node;node=node->NextSiblingElement()) {
		//xmlNode* curr = xpObject->nodesetval->nodeTab[i];
		Person* agent = nullptr;
		unsigned int flagCheck = 0;
		for (TiXmlAttribute* attr=node->FirstAttribute(); attr; attr=attr->Next()) {
			//Read each attribute.
			std::string name = attr->NameTStr();
			std::string value = attr->ValueStr();
			if (name.empty() || value.empty()) {
				return false;
			}
			int valueI;
			std::istringstream(value) >> valueI;

			//Assign it.
			if (name=="id") {
				agent = new Person(valueI);
				if (agentType=="pedestrian") {
					agent->changeRole(new Pedestrian(agent));
				} else if (agentType=="driver") {
					agent->changeRole(new Driver(agent));
				}
				flagCheck |= 1;
			} else if (name=="xPos") {
				agent->xPos.force(valueI);
				flagCheck |= 2;
			} else if (name=="yPos") {
				agent->yPos.force(valueI);
				flagCheck |= 4;
			} else {
				return false;
			}
		}

		if (flagCheck!=7) {
			return false;
		}


		//Save it.
		agents.push_back(agent);
	}

	return true;
}



bool LoadNetworkFromDB(TiXmlElement& parentElem, string& connectionString, map<string, string>& storedProcedures)
{

}



bool LoadXMLBoundariesCrossings(TiXmlDocument& document, const string& parentStr, const string& childStr, map<string, Point2D>& result)
{
	//Quick check.
	if (parentStr!="boundaries" && parentStr!="crossings") {
		return false;
	}
	if (childStr!="boundary" && childStr!="crossing") {
		return false;
	}

	//Build the expression dynamically.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(parentStr).FirstChild(childStr).ToElement();
	if (!node) {
		//Boundaries/crossings are optional
		return true;
	}

	//Move through results
	for (;node; node=node->NextSiblingElement()) {
		//xmlNode* curr = xpObject->nodesetval->nodeTab[i];
		string key;
		Point2D val;
		unsigned int flagCheck = 0;
		for (TiXmlAttribute* attr=node->FirstAttribute(); attr; attr=attr->Next()) {
			//Read each attribute.
			std::string name = attr->NameTStr();
			std::string value = attr->ValueStr();
			if (name.empty() || value.empty()) {
				return false;
			}

			//Assign it.
			if (name=="position") {
				key = value;
				flagCheck |= 1;
			} else {
				int valueI;
				std::istringstream(value) >> valueI;
				if (name=="xPos") {
					val.xPos = valueI;
					flagCheck |= 2;
				} else if (name=="yPos") {
					val.yPos = valueI;
					flagCheck |= 4;
				} else {
					return false;
				}
			}
		}

		if (flagCheck!=7) {
			return false;
		}

		//Save it.
		result[key] = val;
	}

	return true;
}


//Returns the error message, or an empty string if no error.
std::string loadXMLConf(TiXmlDocument& document, std::vector<Agent*>& agents)
{
	//Save granularities: system
	TiXmlHandle handle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("simulation");
	int baseGran = ReadGranularity(handle, "base_granularity");
	int totalRuntime = ReadGranularity(handle, "total_runtime");
	int totalWarmup = ReadGranularity(handle, "total_warmup");

	//Save more granularities
	handle = TiXmlHandle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("granularities");
	int granAgent = ReadGranularity(handle, "agent");
	int granSignal = ReadGranularity(handle, "signal");
	int granPaths = ReadGranularity(handle, "paths");
	int granDecomp = ReadGranularity(handle, "decomp");

	//Check
    if(    baseGran==-1 || totalRuntime==-1 || totalWarmup==-1
    	|| granAgent==-1 || granSignal==-1 || granPaths==-1 || granDecomp==-1) {
        return "Unable to read config file.";
    }

    //Granularity check
    if (granAgent%baseGran != 0) {
    	return "Agent granularity not a multiple of base granularity.";
    }
    if (granSignal%baseGran != 0) {
    	return "Signal granularity not a multiple of base granularity.";
    }
    if (granPaths%baseGran != 0) {
    	return "Path granularity not a multiple of base granularity.";
    }
    if (granDecomp%baseGran != 0) {
    	return "Decomposition granularity not a multiple of base granularity.";
    }
    if (totalRuntime%baseGran != 0) {
    	std::cout <<"  Warning! Total Runtime will be truncated.\n";
    }
    if (totalWarmup%baseGran != 0) {
    	std::cout <<"  Warning! Total Warmup will be truncated.\n";
    }

    //Load ALL agents: drivers and pedestrians.
    //  (Note: Use separate config files if you just want to test one kind of agent.)
    if (!loadXMLAgents(document, agents, "pedestrian")) {
    	return "Couldn't load pedestrians";
    }
    if (!loadXMLAgents(document, agents, "driver")) {
    	return	 "Couldn't load drivers";
    }


    //Check the type of geometry
    handle = TiXmlHandle(&document);
    TiXmlElement* geomElem = handle.FirstChild("config").FirstChild("geometry").ToElement();
    if (geomElem) {
    	const char* geomType = geomElem->Attribute("type");
    	if (geomType && string(geomType) == "simple") {
    		//Load boundaries
    		if (!LoadXMLBoundariesCrossings(document, "boundaries", "boundary", ConfigParams::GetInstance().boundaries)) {
    			return "Couldn't load boundaries";
    		}

    		//Load crossings
    		if (!LoadXMLBoundariesCrossings(document, "crossings", "crossing", ConfigParams::GetInstance().crossings)) {
    			return "Couldn't load crossings";
    		}
    	} else if (geomType && string(geomType) == "aimsun") {
    		//Ensure we're loading from a database
    		const char* geomSrc = geomElem->Attribute("source");
    		if (!geomSrc || "database" != string(geomSrc)) {
    			return "Unknown geometry source: " + (geomSrc?string(geomSrc):"");
    		}

    		//Load the AIMSUM network details
    		map<string, string> storedProcedures; //Of the form "node" -> "get_node()"
    		if (!LoadNetworkFromDB(*geomElem, ConfigParams::GetInstance().connectionString, storedProcedures)) {
    			return "Unable to load database connection settings.";
    		}

    		//Confirm that all stored procedures have been set.
    		if (
    			   storedProcedures.count("node")==0 || storedProcedures.count("section")==0
    			|| storedProcedures.count("turning")==0 || storedProcedures.count("polyline")==0
    		) {
    			return "Not all stored procedures were specified.";
    		}

    		//Actually load it
    		sim_mob::aimsun::Loader::LoadNetwork(ConfigParams::GetInstance().connectionString, storedProcedures, ConfigParams::GetInstance().network);
    	} else {
    		return "Unknown geometry type: " + (geomType?string(geomType):"");
    	}
    }


    //Save
    {
    	ConfigParams& config = ConfigParams::GetInstance();
    	config.baseGranMS = baseGran;
    	config.totalRuntimeTicks = totalRuntime/baseGran;
    	config.totalWarmupTicks = totalWarmup/baseGran;
    	config.granAgentsTicks = granAgent/baseGran;
    	config.granSignalsTicks = granSignal/baseGran;
    	config.granPathsTicks = granPaths/baseGran;
    	config.granDecompTicks = granDecomp/baseGran;
    }

    //Display
    std::cout <<"Config parameters:\n";
    std::cout <<"------------------\n";
    std::cout <<"  Base Granularity: " <<ConfigParams::GetInstance().baseGranMS <<" " <<"ms" <<"\n";
    std::cout <<"  Total Runtime: " <<ConfigParams::GetInstance().totalRuntimeTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Total Warmup: " <<ConfigParams::GetInstance().totalWarmupTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Agent Granularity: " <<ConfigParams::GetInstance().granAgentsTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Signal Granularity: " <<ConfigParams::GetInstance().granSignalsTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Paths Granularity: " <<ConfigParams::GetInstance().granPathsTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Decomp Granularity: " <<ConfigParams::GetInstance().granDecompTicks <<" " <<"ticks" <<"\n";
    if (!ConfigParams::GetInstance().boundaries.empty() || !ConfigParams::GetInstance().crossings.empty()) {
    	std::cout <<"  Boundaries Found: " <<ConfigParams::GetInstance().boundaries.size() <<"\n";
		for (map<string, Point2D>::iterator it=ConfigParams::GetInstance().boundaries.begin(); it!=ConfigParams::GetInstance().boundaries.end(); it++) {
			std::cout <<"    Boundary[" <<it->first <<"] = (" <<it->second.xPos <<"," <<it->second.yPos <<")\n";
		}
		std::cout <<"  Crossings Found: " <<ConfigParams::GetInstance().crossings.size() <<"\n";
		for (map<string, Point2D>::iterator it=ConfigParams::GetInstance().crossings.begin(); it!=ConfigParams::GetInstance().crossings.end(); it++) {
			std::cout <<"    Crossing[" <<it->first <<"] = (" <<it->second.xPos <<"," <<it->second.yPos <<")\n";
		}
    }
    //TODO: Output AIMSUN network.
    std::cout <<"  Agents Initialized: " <<agents.size() <<"\n";
    for (size_t i=0; i<agents.size(); i++) {
    	std::cout <<"    Agent(" <<agents[i]->getId() <<") = " <<agents[i]->xPos.get() <<"," <<agents[i]->yPos.get() <<"\n";
    }
    std::cout <<"------------------\n";

	//No error
	return "";
}

}



//////////////////////////////////////////
// Simple singleton implementation
//////////////////////////////////////////
ConfigParams sim_mob::ConfigParams::instance;
sim_mob::ConfigParams::ConfigParams() {

}
ConfigParams& sim_mob::ConfigParams::GetInstance() {
	return ConfigParams::instance;
}





//////////////////////////////////////////
// Main external method
//////////////////////////////////////////

bool sim_mob::ConfigParams::InitUserConf(const string& configPath, std::vector<Agent*>& agents, std::vector<Region*>& regions,
		          std::vector<TripChain*>& trips, std::vector<ChoiceSet*>& chSets,
		          std::vector<Vehicle*>& vehicles)
{
	//Load our config file into an XML document object.
	TiXmlDocument doc(configPath);
	if (!doc.LoadFile()) {
		std::cout <<"Error loading config file: " <<doc.ErrorDesc() <<std::endl;
		return false;
	}

	//Parse it
	string errorMsg = loadXMLConf(doc, agents);
	if (errorMsg.empty()) {
		std::cout <<"XML config file loaded." <<std::endl;
	} else {
		std::cout <<"Aborting on Config Error: " <<errorMsg <<std::endl;
	}

	//TEMP:
	for (size_t i=0; i<5; i++)
		regions.push_back(new Region(i));
	for (size_t i=0; i<6; i++)
		trips.push_back(new TripChain(i));
	for (size_t i=0; i<15; i++)
		chSets.push_back(new ChoiceSet(i));
	for (size_t i=0; i<10; i++)
		vehicles.push_back(new Vehicle(i));
	if (errorMsg.empty()) {
		std::cout <<"Configuration complete." <<std::endl;
	}


	return errorMsg.empty();

}
