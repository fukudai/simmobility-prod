//This class contains all "primary" classes; i.e., those which are the same as those in the geospatial/ folder.
#pragma once

namespace sim_mob {
namespace xml {


class Point2D_t_pimpl: public virtual Point2D_t_pskel {
public:
	virtual void pre ();
    virtual sim_mob::Point2D post_Point2D_t ();

    virtual void xPos (unsigned int);
    virtual void yPos (unsigned int);

private:
    Point2D model;
};


class lane_t_pimpl: public virtual lane_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::Lane* post_lane_t ();

	virtual void laneID (unsigned long long);
	virtual void width (unsigned int);
	virtual void PolyLine (std::vector<sim_mob::Point2D>);

	virtual void can_go_straight (bool);
	virtual void can_turn_left (bool);
	virtual void can_turn_right (bool);
	virtual void can_turn_on_red_signal (bool);
	virtual void can_change_lane_left (bool);
	virtual void can_change_lane_right (bool);
	virtual void is_road_shoulder (bool);
	virtual void is_bicycle_lane (bool);
	virtual void is_pedestrian_lane (bool);
	virtual void is_vehicle_lane (bool);
	virtual void is_standard_bus_lane (bool);
	virtual void is_whole_day_bus_lane (bool);
	virtual void is_high_occupancy_vehicle_lane (bool);
	virtual void can_freely_park_here (bool);
	virtual void can_stop_here (bool);
	virtual void is_u_turn_allowed (bool);

private:
  sim_mob::Lane model;
};


class connector_t_pimpl: public virtual connector_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,unsigned long> post_connector_t ();

	virtual void laneFrom (unsigned long long);
	virtual void laneTo (unsigned long long);

private:
	std::pair<unsigned long,unsigned long> model;
};


class Multi_Connector_t_pimpl: public virtual Multi_Connector_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,std::set<std::pair<unsigned long,unsigned long> > > post_Multi_Connector_t ();

	virtual void RoadSegment (unsigned long long);
	virtual void Connectors (std::set<std::pair<unsigned long,unsigned long> >);

private:
	//std::string RoadSegment_;
	std::pair<unsigned long,std::set<std::pair<unsigned long,unsigned long> > >  model;
};


class segment_t_pimpl: public virtual segment_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::RoadSegment* post_segment_t ();

	virtual void segmentID (unsigned long long);
	virtual void startingNode (unsigned int);
	virtual void endingNode (unsigned int);
	virtual void maxSpeed (short);
	virtual void Length (unsigned int);
	virtual void Width (unsigned int);
	virtual void originalDB_ID (const ::std::string&);
	virtual void polyline (std::vector<sim_mob::Point2D>);
	virtual void laneEdgePolylines_cached (std::vector<std::vector<sim_mob::Point2D> >);
	virtual void Lanes (std::vector<sim_mob::Lane*>);
	virtual void Obstacles (std::map<sim_mob::centimeter_t,const RoadItem*>);
	virtual void KurbLine (std::vector<sim_mob::Point2D>);

private:
  sim_mob::RoadSegment model;
};



class link_t_pimpl: public virtual link_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::Link* post_link_t ();

	virtual void linkID (unsigned int);
	virtual void roadName (const ::std::string&);
	virtual void StartingNode (unsigned int);
	virtual void EndingNode (unsigned int);
	virtual void Segments (std::pair<std::vector<sim_mob::RoadSegment*>,std::vector<sim_mob::RoadSegment*> >);

private:
	sim_mob::Link model;
};


class Node_t_pimpl: public virtual Node_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::Node* post_Node_t ();

	virtual void nodeID (unsigned int);
	virtual void location (sim_mob::Point2D);
	virtual void linkLoc (unsigned long long);
	virtual void originalDB_ID (const ::std::string&);

	static void RegisterLinkLoc(sim_mob::Node* node, unsigned int link);
	static unsigned int GetLinkLoc(sim_mob::Node* node);
	static std::map<sim_mob::Node*, unsigned int>& GetLinkLocList();

private:
	sim_mob::Node model;
	unsigned int linkLocSaved;

	static std::map<sim_mob::Node*, unsigned int> LinkLocCache;
};




class UniNode_t_pimpl: public virtual UniNode_t_pskel, public ::sim_mob::xml::Node_t_pimpl {
public:
	typedef std::set<std::pair<unsigned long,unsigned long> > LaneConnectSet;
	typedef std::pair<unsigned long,unsigned long> SegmentPair;

	virtual void pre ();
	virtual sim_mob::UniNode* post_UniNode_t ();

	virtual void firstPair (std::pair<unsigned long,unsigned long>);
	virtual void secondPair (std::pair<unsigned long,unsigned long>);
	virtual void Connectors (LaneConnectSet);

	static void RegisterConnectors(sim_mob::UniNode* lane, const LaneConnectSet& connectors);
	static LaneConnectSet GetConnectors(sim_mob::UniNode* lane);

	static void RegisterSegmentPairs(sim_mob::UniNode* lane, const std::pair<SegmentPair, SegmentPair>& pairs);
	static std::pair<SegmentPair, SegmentPair> GetSegmentPairs(sim_mob::UniNode* lane);

private:
	//NOTE: This parameter name shadows Node::model, but this might be the right way to do things anyway.
	sim_mob::UniNode model;

	//Due to a load cycle, we have to save these as integers.
	LaneConnectSet connectors;
	std::pair<SegmentPair, SegmentPair> segmentPairs;

	static std::map<sim_mob::UniNode*, LaneConnectSet> ConnectCache;
	static std::map<sim_mob::UniNode*, std::pair<SegmentPair, SegmentPair> > SegmentPairCache;
};



class intersection_t_pimpl: public virtual intersection_t_pskel, public ::sim_mob::xml::Node_t_pimpl {
public:
	typedef std::map<unsigned long,std::set<std::pair<unsigned long,unsigned long> > > LaneConnectSet;
	typedef std::set<unsigned long> RoadSegmentSet;

	virtual void pre ();
	virtual sim_mob::MultiNode* post_intersection_t ();

	virtual void roadSegmentsAt (RoadSegmentSet);
	virtual void Connectors (const LaneConnectSet&);
	virtual void ChunkLengths ();
	virtual void Offsets ();
	virtual void Separators ();
	virtual void additionalDominantLanes ();
	virtual void additionalSubdominantLanes ();
	virtual void domainIslands ();

	static void RegisterConnectors(sim_mob::MultiNode* intersection, const LaneConnectSet& connectors);
	static LaneConnectSet GetConnectors(sim_mob::MultiNode* intersection);

	static void RegisterSegmentsAt(sim_mob::MultiNode* intersection, const RoadSegmentSet& segmentsAt);
	static RoadSegmentSet GetSegmentsAt(sim_mob::MultiNode* intersection);

private:
	sim_mob::Intersection model;
	LaneConnectSet connectors;
	RoadSegmentSet segmentsAt;

	static std::map<sim_mob::MultiNode*, LaneConnectSet> ConnectCache;
	static std::map<sim_mob::MultiNode*, RoadSegmentSet> SegmentsAtCache;
};



class GeoSpatial_t_pimpl: public virtual GeoSpatial_t_pskel {
public:
	virtual void pre ();
	virtual void post_GeoSpatial_t ();

	virtual void RoadNetwork ();
};



class roundabout_t_pimpl: public virtual roundabout_t_pskel, public ::sim_mob::xml::Node_t_pimpl {
public:
	virtual void pre ();
	virtual sim_mob::MultiNode* post_roundabout_t ();

	virtual void roadSegmentsAt (std::set<unsigned long>);
	virtual void Connectors (const std::map<unsigned long,std::set<std::pair<unsigned long,unsigned long> > >&);
	virtual void ChunkLengths ();
	virtual void Offsets ();
	virtual void Separators ();
	virtual void addDominantLane ();
	virtual void roundaboutDominantIslands (float);
	virtual void roundaboutNumberOfLanes (int);
	virtual void entranceAngles ();
};


class RoadItem_t_pimpl: public virtual RoadItem_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,sim_mob::RoadItem*> post_RoadItem_t ();

	virtual void id (unsigned long long);
	virtual void Offset (unsigned short);
	virtual void start (sim_mob::Point2D);
	virtual void end (sim_mob::Point2D);

private:
	sim_mob::RoadItem model;
	unsigned short offset;

	/*unsigned long id_;
	unsigned short Offset_;
	sim_mob::Point2D start_;
	sim_mob::Point2D end_;*/
};


class BusStop_t_pimpl: public virtual BusStop_t_pskel, public ::sim_mob::xml::RoadItem_t_pimpl {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,sim_mob::BusStop*> post_BusStop_t ();

	virtual void xPos (double);
	virtual void yPos (double);
	virtual void lane_location (unsigned long long);
	virtual void is_terminal (bool);
	virtual void is_bay (bool);
	virtual void has_shelter (bool);
	virtual void busCapacityAsLength (unsigned int);
	virtual void busstopno (const ::std::string&);

private:
	sim_mob::BusStop model;
};


class ERP_Gantry_t_pimpl: public virtual ERP_Gantry_t_pskel, public ::sim_mob::xml::RoadItem_t_pimpl {
public:
	virtual void pre ();
	virtual void post_ERP_Gantry_t ();

	virtual void ERP_GantryID (const ::std::string&);
};


class crossing_t_pimpl: public virtual crossing_t_pskel, public ::sim_mob::xml::RoadItem_t_pimpl {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,sim_mob::Crossing*> post_crossing_t ();

	virtual void nearLine (std::pair<sim_mob::Point2D,sim_mob::Point2D>);
	virtual void farLine (std::pair<sim_mob::Point2D,sim_mob::Point2D>);

private:
	sim_mob::Crossing model;
};


class RoadBump_t_pimpl: public virtual RoadBump_t_pskel, public ::sim_mob::xml::RoadItem_t_pimpl {
public:
	virtual void pre ();
	virtual void post_RoadBump_t ();

	virtual void roadBumpID (const ::std::string&);
	virtual void segmentID (unsigned long long);
};


class RoadNetwork_t_pimpl: public virtual RoadNetwork_t_pskel {
public:
	RoadNetwork_t_pimpl(); //TODO: No constructors!

	virtual void pre ();
	virtual void post_RoadNetwork_t ();

	virtual void Nodes ();
	virtual void Links (std::vector<sim_mob::Link*>);

private:
  sim_mob::RoadNetwork& modelRef;
};


class Plan_t_pimpl: public virtual Plan_t_pskel {
public:
	virtual void pre ();
	virtual void post_Plan_t ();

	virtual void planID (unsigned char);
	virtual void PhasePercentage (double);
};


}}
