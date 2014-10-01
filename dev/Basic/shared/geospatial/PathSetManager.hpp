/*
 * PathSetManager.hpp
 *
 *  Created on: May 6, 2013
 *      Author: Max
 *      Author: Vahid
 */

#pragma once

#include "geospatial/streetdir/KShortestPathImpl.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/Link.hpp"
#include "entities/Person.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "util/Profiler.hpp"
#include "util/Cache.hpp"
#include "util/Utils.hpp"

namespace sim_mob
{
namespace batched {
class ThreadPool;
}

///	Debug Method to print WayPoint based paths
void printWPpath(const std::vector<WayPoint> &wps , const sim_mob::Node* startingNode = 0);

/**
 * This class is used to store, retrieve, cache different parameters used in the pathset generation
 */
class PathSetParam {
private:
	PathSetParam();
	static PathSetParam *instance_;

public:
	static PathSetParam *getInstance();

	/// Retrieve 'ERP' and 'link travel time' information from Database
	void getDataFromDB();

	///	get RoadSegment from AimsunId
	sim_mob::RoadSegment* getRoadSegmentByAimsunId(const std::string id);

	///	insert an entry into singlepath table in the database
	void storeSinglePath(soci::session& sql,std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,const std::string singlePathTableName);

	///	insert an entry into pathset table in the database
	void storePathSet(soci::session& sql,std::map<std::string,boost::shared_ptr<sim_mob::PathSet> >& psPool,const std::string pathSetTableName);

	///	set the table name used to store temporary travel time information
	void setTravleTimeTmpTableName(const std::string& value);

	///	create the table used to store temporary travel time information
	bool createTravelTimeTmpTable();

	///	drop the table used to store temporary travel time information
	bool dropTravelTimeTmpTable();

	/// create the table used to store realtime travel time information
	bool createTravelTimeRealtimeTable();

	///	get the average travel time of a segment within a time range from 'real time' or 'default' source
	double getAverageTravelTimeBySegIdStartEndTime(std::string id,sim_mob::DailyTime startTime,sim_mob::DailyTime endTime);

	///	get the travel time of a segment from 'default' source
	double getDefaultTravelTimeBySegId(std::string id);

	///	get travel time of a segment in a specific time from 'real time' or 'default' source
	double getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime);

	///	return a waypoint wrapped segment(either create,cache and return OR find in the cache and return)
	sim_mob::WayPoint* getWayPointBySeg(const sim_mob::RoadSegment* seg);

	///	return cached node given its id
	sim_mob::Node* getCachedNode(std::string id);

	double getHighwayBias() { return highway_bias; }

	///	initialize pathset parameters with some fixed values
	void initParameters();
	///	return the current rough size of the class
	uint32_t getSize();
	///	pathset parameters
	double bTTVOT;
	double bCommonFactor;
	double bLength;
	double bHighway;
	double bCost;
	double bSigInter;
	double bLeftTurns;
	double bWork;
	double bLeisure;
	double highway_bias;
	double minTravelTimeParam;
	double minDistanceParam;
	double minSignalParam;
	double maxHighwayParam;

	///	store all segs <aimsun id ,seg>
	std::map<std::string,sim_mob::RoadSegment*> segPool;

	///	<seg , value>
	std::map<const sim_mob::RoadSegment*,sim_mob::WayPoint*> wpPool;

	///	store all nodes <id ,node>
	std::map<std::string,sim_mob::Node*> nodePool;

	///	store all multi nodes in the map
	const std::vector<sim_mob::MultiNode*>  &multiNodesPool;

	///	store all uni nodes
	const std::set<sim_mob::UniNode*> & uniNodesPool;

	///	ERP surcharge  information <gantryNo , value=ERP_Surcharge with same No diff time stamp>
	std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_SurchargePool;

	///	ERP Zone information <gantryNo, ERP_Gantry_Zone>
	std::map<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_ZonePool;

	///	ERP section <aim-sun id , ERP_Section>
	std::map<std::string,sim_mob::ERP_Section*> ERP_SectionPool;

	///	information of "Segment" default travel time <segment aim-sun id ,Link_default_travel_time with diff time stamp>
	std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > segmentDefaultTravelTimePool;

	///	information of "Segment" reatravel time <segment aim-sun id ,Link_default_travel_time with diff time stamp>
	std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > segmentRealTimeTravelTimePool;

	///	simmobility's road network
	const sim_mob::RoadNetwork& roadNetwork;

	/// table store travel time ,used to calculate pathset size
	std::string pathSetTravelTimeRealTimeTableName;

	/// table store travel time generated by runtime, use for next simulation
	std::string pathSetTravelTimeTmpTableName;

};
///////////////////////////////////////////////////////////////////////////////////////////////////////
class ERP_Gantry_Zone
{
public:
	ERP_Gantry_Zone() {}
	ERP_Gantry_Zone(ERP_Gantry_Zone &src):gantryNo(src.gantryNo),zoneId(src.zoneId) {}
	std::string gantryNo;
	std::string zoneId;
};

class ERP_Section
{
public:
	ERP_Section() {}
	ERP_Section(ERP_Section &src);
	int section_id;
	int ERP_Gantry_No;
	std::string ERP_GantryNoStr;
	OpaqueProperty<int> originalSectionDB_ID;  // seg aim-sun id ,rs->originalDB_ID.setProps("aimsun-id", currSec->id);
};

class ERP_Surcharge
{
public:
	ERP_Surcharge() {}
	ERP_Surcharge(ERP_Surcharge& src):gantryNo(src.gantryNo),startTime(src.startTime),endTime(src.endTime),rate(src.rate),
			vehicleTypeId(src.vehicleTypeId),vehicleTypeDesc(src.vehicleTypeDesc),day(src.day),
			startTime_DT(sim_mob::DailyTime(src.startTime)),endTime_DT(sim_mob::DailyTime(src.endTime)){}
	std::string gantryNo;
	std::string startTime;
	std::string endTime;
	sim_mob::DailyTime startTime_DT;
	sim_mob::DailyTime endTime_DT;
	double rate;
	int vehicleTypeId;
	std::string vehicleTypeDesc;
	std::string day;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LinkTravelTime
{
public:
	LinkTravelTime() {};
	LinkTravelTime(LinkTravelTime& src);
	int linkId;
	std::string startTime;
	std::string endTime;
	sim_mob::DailyTime startTime_DT;
	sim_mob::DailyTime endTime_DT;
	double travelTime;
	OpaqueProperty<int> originalSectionDB_ID;

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// length of a path with segments in meter
inline double generateSinglePathLengthPT(std::vector<WayPoint>& wp)
{
	double res=0;
	for(int i=0;i<wp.size();++i)
	{
		WayPoint &w = wp[i];
		if (w.type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w.roadSegment_;
			res += seg->length;
		}
	}
	return res/100.0; //meter
}

enum TRIP_PURPOSE
{
	work = 1,
	leisure = 2
};

template<typename T>
std::string toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

/// Roadsegment-Person
typedef std::multimap<const sim_mob::RoadSegment*, const sim_mob::Person* > SGPER ;// Roadsegment-Person  :)

/*****************************************************
 * ****** Path Set Manager ***************************
 * ***************************************************
 */
class PathSetManager {

public:
	static PathSetManager* getInstance()
	{
		if(!instance_)
		{
			instance_ = new PathSetManager();
		}
		return instance_;
	}
public:
	bool generateAllPathSetWithTripChain2();
	///	generate shortest path information
	sim_mob::SinglePath *  generateSinglePathByFromToNodes3( const sim_mob::Node *fromNode, const sim_mob::Node *toNode,
			   std::set<std::string> & duplicatePath, const std::set<const sim_mob::RoadSegment*> & excludedSegs=std::set<const sim_mob::RoadSegment*>());

	///	generate a path based on shortest travel time
	sim_mob::SinglePath* generateShortestTravelTimePath(const sim_mob::Node *fromNode, const sim_mob::Node *toNode,
			std::set<std::string>& duplicateChecker, sim_mob::TimeRange tr = sim_mob::MorningPeak,
			const sim_mob::RoadSegment* excludedSegs=NULL, int random_graph_idx=0);

	bool isUseCacheMode() { return false;/*isUseCache;*/ }//todo: take care of this later
	double getUtilityBySinglePath(sim_mob::SinglePath* sp);
	std::vector<WayPoint> generateBestPathChoice2(const sim_mob::SubTrip* st);
	/**
	 * generate set of path choices for a given suntrip, and then return the best of them
	 * \param st input subtrip
	 * \param res output path generated
	 * \param excludedSegs input list segments to be excluded from the target set
	 * \param isUseCache is using the cache allowed
	 */
	 bool generateBestPathChoiceMT(std::vector<sim_mob::WayPoint>& res, const sim_mob::SubTrip* st, const std::set<const sim_mob::RoadSegment*> & excludedSegs=std::set<const sim_mob::RoadSegment*>(), bool isUseCache = true);

	/**
	 * generate all the paths for a person given its subtrip(OD)
	 * \param per input agent applying to get the path
	 * \param st input subtrip
	 * \param res output path generated
	 * \param excludedSegs input list segments to be excluded from the target set
	 * \param isUseCache is using the cache allowed
	 */
	bool generateAllPathChoicesMT(boost::shared_ptr<sim_mob::PathSet> &ps, const std::set<const sim_mob::RoadSegment*> & excludedSegs=std::set<const sim_mob::RoadSegment*>());

	///	generate travel time required to complete a path represented by different singlepath objects
	void generateTravelTimeSinglePathes(const sim_mob::Node *fromNode, const sim_mob::Node *toNode, std::set<std::string>& duplicateChecker,boost::shared_ptr<sim_mob::PathSet> &ps_);

	void generatePathesByLinkElimination(std::vector<WayPoint>& path,std::set<std::string>& duplicateChecker,boost::shared_ptr<sim_mob::PathSet> &ps_,const sim_mob::Node* fromNode,const sim_mob::Node* toNode);

	void generatePathesByTravelTimeLinkElimination(std::vector<WayPoint>& path, std::set<std::string>& duplicateChecker, boost::shared_ptr<sim_mob::PathSet> &ps_,const sim_mob::Node* fromNode,const sim_mob::Node* toNode,	sim_mob::TimeRange tr);

	bool getBestPathChoiceFromPathSet(boost::shared_ptr<sim_mob::PathSet> &ps, const std::set<const sim_mob::RoadSegment *> & excludedSegs =  std::set<const sim_mob::RoadSegment *>());

	///	initialize various(mainly utility) paramenters
	void initParameters();

	/// one of the main PathSetManager interfaces used to return a path for the current OD of the given person.
	std::vector<WayPoint> getPathByPerson(const sim_mob::Person* per,const sim_mob::SubTrip &subTrip);

	///	calculate travel time of a path
	double getTravelTime(sim_mob::SinglePath *sp);

	///	get travel time of a segment
	double getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime);

	void setCSVFileName();

	bool insertTravelTime2TmpTable(sim_mob::LinkTravelTime& data);

	bool copyTravelTimeDataFromTmp2RealtimeTable();

	void init();

	///clears various cache containers
	void clearPools();

	void setScenarioName(std::string& name){ scenarioName = name; }

	void insertFromTo_BestPath_Pool(std::string& id ,std::vector<WayPoint>& value);

	bool getCachedBestPath(std::string id, std::vector<WayPoint> & value);

	void cacheODbySegment(const sim_mob::Person*,const SubTrip *,std::vector<WayPoint> &);

	const std::pair<SGPER::const_iterator,SGPER::const_iterator > getODbySegment(const sim_mob::RoadSegment* segment) const;

	///	handle messages sent to pathset manager using message bus
	void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	///	get system registered incidents(changes in segment capacities)
	std::set<const sim_mob::RoadSegment*> &getIncidents();

	///insert into incident list
	void inserIncidentList(const sim_mob::RoadSegment*);

	///	get the database session used for this thread
	static const boost::shared_ptr<soci::session> & getSession();
	///basically delete all the dynamically allocated memories, in addition to some more cleanups
	void clearSinglePaths(boost::shared_ptr<sim_mob::PathSet> &ps);
	///cache the generated pathset. returns true upon successful insertion
	bool cachePathSet(boost::shared_ptr<sim_mob::PathSet> &ps);
	/**
	 * searches for a pathset in the cache.
	 * \param key indicates the input key
	 * \param value the result of the search
	 * returns true/false to indicate if the search has been successful
	 */
	bool findCachedPathSet(std::string key, boost::shared_ptr<sim_mob::PathSet> &value);
	///cache the generated pathset. returns true upon successful insertion
	bool cachePathSet_orig(boost::shared_ptr<sim_mob::PathSet> &ps);
	/**
	 * searches for a pathset in the cache.
	 * \param key indicates the input key
	 * \param value the result of the search
	 * returns true/false to indicate if the search has been successful
	 */
	bool findCachedPathSet_orig(std::string key, boost::shared_ptr<sim_mob::PathSet> &value);

	///cache the generated pathset. returns true upon successful insertion
	bool cachePathSet_LRU(boost::shared_ptr<sim_mob::PathSet> &ps);
	/**
	 * searches for a pathset in the cache.
	 * \param key indicates the input key
	 * \param value the result of the search
	 * returns true/false to indicate if the search has been successful
	 */
	bool findCachedPathSet_LRU(std::string key, boost::shared_ptr<sim_mob::PathSet> &value);

	///	returns the raugh size of object in Bytes
	uint32_t getSize();

	PathSetManager();
	~PathSetManager();

private:
	static PathSetManager *instance_;

	///	link to street directory
	StreetDirectory& stdir;

	///	link to pathset paramaters
	PathSetParam *pathSetParam;

	///	is caching on
	bool isUseCache;

	///	list of incident currently happening in the system.(incident manager has the original copy
	std::set<const sim_mob::RoadSegment*> currIncidents;
	///	protect access to incidents list
	boost::shared_mutex mutexIncident;

	///	stores the name of database's pathset table
	const std::string &pathSetTableName;

	///	stores the name of database's singlepath table
	const std::string &singlePathTableName;

	///	stores the name of database's function operating on the pathset and singlepath tables
	const std::string &dbFunction;

	///every thread which invokes db related parts of pathset manages, should have its own connection to the database
	static std::map<boost::thread::id, boost::shared_ptr<soci::session > > cnnRepo;

	///	static sim_mob::Logger profiler;
	static boost::shared_ptr<sim_mob::batched::ThreadPool> threadpool_;

	boost::shared_mutex cachedPathSetMutex;

	std::map<std::string, boost::shared_ptr<sim_mob::PathSet> > cachedPathSet;//same as pathSetPool, used in a separate scenario //todo later use only one of the caches, cancel the other one

	///	Yet another cache
	sim_mob::LRU_Cache<std::string, boost::shared_ptr<PathSet> > cacheLRU;
	///	contains arbitrary description usually to indicating which configuration file the generated data has originated from
	std::string scenarioName;

	/// cache the best chosen path
	std::map<std::string ,std::vector<WayPoint> > fromto_bestPath;

	///	used to avoid entering duplicate "HAS_PATH=-1" pathset entries into PathSet. It will be removed once the cache and/or proper DB functions are in place
	std::set<std::string> tempNoPath;

	///a cache to help answer this question: a given road segment is within which path(s)
	SGPER pathSegments;

	///	file name used to store realtime data
	std::string csvFileName;

	///	file stream used to store realtime data
	std::ofstream csvFile;

	///	link to shortest path implementation
	sim_mob::K_ShortestPathImpl *kshortestImpl;

	///	different utility parameters
	double bTTVOT;
	double bCommonFactor;
	double bLength;
	double bHighway;
	double bCost;
	double bSigInter;
	double bLeftTurns;
	double bWork;
	double bLeisure;
	double highway_bias;
	double minTravelTimeParam;
	double minDistanceParam;
	double minSignalParam;
	double maxHighwayParam;

};
/*****************************************************
 ******************* Single Path *********************
 *****************************************************
 */
class SinglePath
{
public:
	SinglePath() : purpose(work),utility(0.0),pathSize(0.0),travelCost(0.0),
	signal_number(0.0),right_turn_number(0.0),length(0.0),travleTime(0.0),highWayDistance(0.0),
	isMinTravelTime(0),isMinDistance(0),isMinSignal(0),isMinRightTurn(0),isMaxHighWayUsage(0),isShortestPath(0){}
	SinglePath(const SinglePath &source);
	void init(std::vector<WayPoint>& wpPools);
	void clear();
	std::vector<WayPoint> shortestWayPointpath;
	std::set<const RoadSegment*> shortestSegPath;
	boost::shared_ptr<sim_mob::PathSet>pathSet; // parent
	const sim_mob::RoadSegment* excludeSeg; // can be null
	const sim_mob::Node *fromNode;
	const sim_mob::Node *toNode;

	double highWayDistance;
	bool isMinTravelTime;
	bool isMinDistance;
	bool isMinSignal;
	bool isMinRightTurn;
	bool isMaxHighWayUsage;
	bool isShortestPath;

	bool isNeedSave2DB;
	std::string id;   //id: seg1id_seg2id_seg3id
	std::string pathset_id;
	double utility;
	double pathSize;
	double travelCost;
	int signal_number;
	int right_turn_number;
	std::string scenario;
	double length;
	double travleTime;
	sim_mob::TRIP_PURPOSE purpose;

	SinglePath(SinglePath *source);
	~SinglePath();

	 bool operator() (const SinglePath* lhs, const SinglePath* rhs) const
	 {
		 return lhs->id<rhs->id;
	 }
	///	returns the raugh size of object in Bytes
	uint32_t getSize();
	///does these SinglePath include the any of given RoadSegment(s)
	bool includesRoadSegment(const std::set<const sim_mob::RoadSegment*> &segs);

};

/*****************************************************
 ******************* Path Set ************************
 *****************************************************
 */
class PathSet
{
public:
	PathSet():hasPath(0) {};
	PathSet(const sim_mob::Node *fn,const sim_mob::Node *tn) : fromNode(fn),toNode(tn),logsum(0),hasPath(0) {}
	PathSet(const boost::shared_ptr<sim_mob::PathSet> &ps);
	~PathSet();
	///	returns the raugh size of object in Bytes
	uint32_t getSize();
	bool isInit;
	bool hasBestChoice;
	std::vector<WayPoint> *bestWayPointpath;  //best choice
	const sim_mob::Node *fromNode;
	const sim_mob::Node *toNode;
	std::string personId; //person id
	std::string tripId; // trip item id
	SinglePath* oriPath;  // shortest path with all segments
	std::map<std::string,sim_mob::SinglePath*> SinglePathPool;
	std::set<sim_mob::SinglePath*, sim_mob::SinglePath> pathChoices;
	bool isNeedSave2DB;
	double logsum;
	const sim_mob::SubTrip* subTrip; // pathset use info of subtrip to generate all things
	std::string id;
	std::string fromNodeId;
	std::string toNodeId;
	std::string singlepath_id;
	std::string excludedPaths;
	std::string scenario;
	int hasPath;
	PathSetManager *psMgr;

	PathSet(boost::shared_ptr<sim_mob::PathSet> &ps);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline double getTravelCost2(sim_mob::SinglePath *sp)
{

//	sim_mob::Logger::log("path_set").prof("getTravelCost2").tick();
	double res=0.0;
	double ts=0.0;
	if(!sp) {
		sim_mob::Logger::log("path_set") << "gTC: sp is empty" << std::endl;
	}
	sim_mob::DailyTime trip_startTime = sp->pathSet->subTrip->startTime;
	for(std::set<const RoadSegment*>::iterator it1 = sp->shortestSegPath.begin(); it1 != sp->shortestSegPath.end(); it1++)
	{
		std::string seg_id = (*it1)->originalDB_ID.getLogItem();
		std::map<std::string,sim_mob::ERP_Section*>::iterator it = sim_mob::PathSetParam::getInstance()->ERP_SectionPool.find(seg_id);
		//get travel time to this segment
		double t = sim_mob::PathSetParam::getInstance()->getTravelTimeBySegId(seg_id,trip_startTime);
		ts += t;
		trip_startTime = trip_startTime + sim_mob::DailyTime(ts*1000);
		if(it!=sim_mob::PathSetParam::getInstance()->ERP_SectionPool.end())
		{
			sim_mob::ERP_Section* erp_section = (*it).second;
			std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >::iterator itt =
					sim_mob::PathSetParam::getInstance()->ERP_SurchargePool.find(erp_section->ERP_GantryNoStr);
			if(itt!=sim_mob::PathSetParam::getInstance()->ERP_SurchargePool.end())
			{
				std::vector<sim_mob::ERP_Surcharge*> erp_surcharges = (*itt).second;
				for(int i=0;i<erp_surcharges.size();++i)
				{
					sim_mob::ERP_Surcharge* s = erp_surcharges[i];
					if( s->startTime_DT.isBeforeEqual(trip_startTime) && s->endTime_DT.isAfter(trip_startTime) &&
							s->vehicleTypeId == 1 && s->day == "Weekdays")
					{
						res += s->rate;
					}
				}
			}
		}
	}
//	sim_mob::Logger::log("path_set").prof("getTravelCost2").tick(true);
	return res;
}

inline std::string makeWaypointsetString(std::vector<WayPoint>& wp)
{
	std::string str;
	if(wp.size()==0)
	{
		sim_mob::Logger::log("path_set")<<"warning: empty input for makeWaypointsetString"<<std::endl;
	}

	for(std::vector<WayPoint>::iterator it = wp.begin(); it != wp.end(); it++)
	{
		if (it->type_ == WayPoint::ROAD_SEGMENT)
		{
			std::string tmp = it->roadSegment_->originalDB_ID.getLogItem();
			str += sim_mob::Utils::getNumberFromAimsunId(tmp) + ",";
		} // if ROAD_SEGMENT
	}

	if(str.size()<1)
	{
		// when same f,t node, it happened
		sim_mob::Logger::log("path_set")<<"warning: empty output makeWaypointsetString id"<<std::endl;
	}
	return str;
}

inline double generateSinglePathLength(std::vector<WayPoint*>& wp) // unit is meter
{
	double res=0;
	for(int i=0;i<wp.size();++i)
	{
		WayPoint* w = wp[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			res += seg->length;
		}
	}
	return res/100.0; //meter
}

void generatePathSizeForPathSet2(boost::shared_ptr<sim_mob::PathSet> &ps,bool isUseCache=true);


inline size_t getLaneIndex2(const Lane* l){
	if (l) {
		const RoadSegment* r = l->getRoadSegment();
		std::vector<sim_mob::Lane*>::const_iterator it( r->getLanes().begin()), itEnd(r->getLanes().end());
		for (size_t i = 0; it != itEnd; it++, i++) {
			if (*it == l) {
				return i;
			}
		}
	}
	return -1; //NOTE: This might not do what you expect! ~Seth
}

void calculateRightTurnNumberAndSignalNumberByWaypoints(sim_mob::SinglePath *sp);

inline double calculateHighWayDistance(sim_mob::SinglePath *sp)
{
	double res=0;
	if(!sp) return 0.0;
	for(int i=0;i<sp->shortestWayPointpath.size();++i)
	{
		sim_mob::WayPoint& w = sp->shortestWayPointpath[i];
		if (w.type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w.roadSegment_;
			if(seg->maxSpeed >= 60)
			{
				res += seg->length;
			}
		}
	}
	return res/100.0; //meter
}
static unsigned int seed = 0;
inline float gen_random_float(float min, float max)
{
    boost::mt19937 rng;
    rng.seed(static_cast<unsigned int>(std::time(0) + (++seed)));
    boost::uniform_real<float> u(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > gen(rng, u);
    return gen();
}

}//namespace
