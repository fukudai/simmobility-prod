// Not copyrighted - public domain.
//
// This sample parser implementation was generated by CodeSynthesis XSD,
// an XML Schema to C++ data binding compiler. You may use it in your
// programs without any restrictions.
//

#include "geo8-pimpl.hpp"
#include <cstdio>
#include <iostream>

namespace geo
{
  // SegmentType_t_pimpl
  //

  void SegmentType_t_pimpl::
  pre ()
  {
  }

  std::string SegmentType_t_pimpl::
  post_SegmentType_t ()
  {
    const ::std::string& v (post_string ());

    // TODO
    //
    // return ... ;
  }

  // ObstacleType_t_pimpl
  //

  void ObstacleType_t_pimpl::
  pre ()
  {
  }

  void ObstacleType_t_pimpl::
  post_ObstacleType_t ()
  {
    const ::std::string& v (post_string ());

    std::cout << "ObstacleType_t: " << v << std::endl;
  }

  // LaneType_t_pimpl
  //

  void LaneType_t_pimpl::
  pre ()
  {
  }

  void LaneType_t_pimpl::
  post_LaneType_t ()
  {
    const ::std::string& v (post_string ());

    std::cout << "LaneType_t: " << v << std::endl;
  }

  // obstacle_t_pimpl
  //

  void obstacle_t_pimpl::
  pre ()
  {
  }

  void obstacle_t_pimpl::
  obstacleID (const ::std::string& obstacleID)
  {
    std::cout << "obstacleID: " << obstacleID << std::endl;
  }

  void obstacle_t_pimpl::
  obstacleType ()
  {
  }

  sim_mob::RoadItem* obstacle_t_pimpl::
  post_obstacle_t ()
  {
    // TODO
    //
    // return ... ;
  }

  // obstacles_t_pimpl
  //

  void obstacles_t_pimpl::
  pre ()
  {
  }

  void obstacles_t_pimpl::
  obstacle (sim_mob::RoadItem* obstacle)
  {
    // TODO
    //
  }

  std::vector<sim_mob::RoadItem> obstacles_t_pimpl::
  post_obstacles_t ()
  {
    // TODO
    //
    // return ... ;
  }

  // Point2D_t_pimpl
  //

  void Point2D_t_pimpl::
  pre ()
  {
  }

  void Point2D_t_pimpl::
  xPos (unsigned int xPos)
  {
    std::cout << "xPos: " << xPos << std::endl;
    point2D.setX(xPos);
  }

  void Point2D_t_pimpl::
  yPos (unsigned int yPos)
  {
    std::cout << "yPos: " << yPos << std::endl;
    point2D.setY(yPos);
  }

  sim_mob::Point2D Point2D_t_pimpl::
  post_Point2D_t ()
  {
	  return point2D;
  }

  // PolyPoint_t_pimpl
  //

  void PolyPoint_t_pimpl::
  pre ()
  {
	  std::cout << "PolyPoint_t_pimpl::pre\n";
  }

  void PolyPoint_t_pimpl::
  pointID (const ::std::string& pointID)
  {
    std::cout << "PolyPoint_t_pimpl::pointID: " << pointID << std::endl;
  }

  void PolyPoint_t_pimpl::
  location (sim_mob::Point2D location)
  {
	  std::cout << "PolyPoint_t_pimpl::location\n";
	  point2D = location;
  }

  sim_mob::Point2D PolyPoint_t_pimpl::
  post_PolyPoint_t ()
  {
	  return point2D;
  }

  // PolyLine_t_pimpl
  //

  void PolyLine_t_pimpl::
  pre ()
  {
  }

  void PolyLine_t_pimpl::
  PolyPoint (sim_mob::Point2D PolyPoint)
  {
	  polyLine.push_back(PolyPoint);
  }

  std::vector<sim_mob::Point2D> PolyLine_t_pimpl::
  post_PolyLine_t ()
  {
     return polyLine ;
  }

  // lane_t_pimpl
  //

  void lane_t_pimpl::
  pre ()
  {
	  this->lane = new sim_mob::Lane();
  }

  void lane_t_pimpl::
  laneID (const ::std::string& laneID)
  {
	  this->lane->laneID_ = atoi(laneID.c_str());
  }

  void lane_t_pimpl::
  width (unsigned int width)
  {
	  this->lane->width_ = width;
  }

  void lane_t_pimpl::
  can_go_straight (bool can_go_straight)
  {
	  this->lane->can_go_straight(can_go_straight);
  }

  void lane_t_pimpl::
  can_turn_left (bool can_turn_left)
  {
	  this->lane->can_turn_left(can_turn_left);
  }

  void lane_t_pimpl::
  can_turn_right (bool can_turn_right)
  {
	  this->lane->can_turn_right(can_turn_right);
  }

  void lane_t_pimpl::
  can_turn_on_red_signal (bool can_turn_on_red_signal)
  {
	  this->lane->can_turn_on_red_signal(can_turn_on_red_signal);
  }

  void lane_t_pimpl::
  can_change_lane_left (bool can_change_lane_left)
  {
	  this->lane->can_change_lane_left(can_change_lane_left);
  }

  void lane_t_pimpl::
  can_change_lane_right (bool can_change_lane_right)
  {
	  this->lane->can_change_lane_right(can_change_lane_right);
  }

  void lane_t_pimpl::
  is_road_shoulder (bool is_road_shoulder)
  {
	  this->lane->is_road_shoulder(is_road_shoulder);
  }

  void lane_t_pimpl::
  is_bicycle_lane (bool is_bicycle_lane)
  {
	  this->lane->is_bicycle_lane(is_bicycle_lane);
  }

  void lane_t_pimpl::
  is_pedestrian_lane (bool is_pedestrian_lane)
  {
	  this->lane->is_pedestrian_lane(is_pedestrian_lane);
  }

  void lane_t_pimpl::
  is_vehicle_lane (bool is_vehicle_lane)
  {
	  this->lane->is_vehicle_lane(is_vehicle_lane);
  }

  void lane_t_pimpl::
  is_standard_bus_lane (bool is_standard_bus_lane)
  {
	  this->lane->is_standard_bus_lane(is_standard_bus_lane);
  }

  void lane_t_pimpl::
  is_whole_day_bus_lane (bool is_whole_day_bus_lane)
  {
	  this->lane->is_whole_day_bus_lane(is_whole_day_bus_lane);
  }

  void lane_t_pimpl::
  is_high_occupancy_vehicle_lane (bool is_high_occupancy_vehicle_lane)
  {
	  this->lane->is_high_occupancy_vehicle_lane(is_high_occupancy_vehicle_lane);
  }

  void lane_t_pimpl::
  can_freely_park_here (bool can_freely_park_here)
  {
	  this->lane->can_freely_park_here(can_freely_park_here);
  }

  void lane_t_pimpl::
  can_stop_here (bool can_stop_here)
  {
	  this->lane->can_stop_here(can_stop_here);
  }

  void lane_t_pimpl::
  is_u_turn_allowed (bool is_u_turn_allowed)
  {
	  this->lane->is_u_turn_allowed(is_u_turn_allowed);
  }

  void lane_t_pimpl::
  PolyLine (std::vector<sim_mob::Point2D> PolyLine)
  {
	  this->lane->polyline_ = PolyLine;
  }

  sim_mob::Lane* lane_t_pimpl::
  post_lane_t ()
  {
	  std::cout<< "Lane returning\n";
    return this->lane;
  }

  // connector_t_pimpl
  //

  void connector_t_pimpl::
  pre ()
  {
  }

  void connector_t_pimpl::
  laneFrom (const ::std::string& laneFrom)
  {
    std::cout << "laneFrom: " << laneFrom << std::endl;
  }

  void connector_t_pimpl::
  laneTo (const ::std::string& laneTo)
  {
    std::cout << "laneTo: " << laneTo << std::endl;
  }

  sim_mob::LaneConnector* connector_t_pimpl::
  post_connector_t ()
  {
    // TODO
    //
    // return ... ;
  }

  // connectors_t_pimpl
  //

  void connectors_t_pimpl::
  pre ()
  {
  }

  void connectors_t_pimpl::
  Connector (sim_mob::LaneConnector* Connector)
  {
    // TODO
    //
  }

  void connectors_t_pimpl::
  post_connectors_t ()
  {
  }

  // fwdBckSegments_t_pimpl
  //

  void fwdBckSegments_t_pimpl::
  pre ()
  {
  }

  void fwdBckSegments_t_pimpl::
  Segment (sim_mob::RoadSegment* Segment)
  {
	  Segments.push_back(Segment);
  }

  std::vector<sim_mob::RoadSegment*> fwdBckSegments_t_pimpl::
  post_fwdBckSegments_t ()
  {
	  std::cout << "In post_fwdBckSegments_t\n";
	  return Segments;
  }

  // RoadSegmentsAt_t_pimpl
  //

  void RoadSegmentsAt_t_pimpl::
  pre ()
  {
  }

  void RoadSegmentsAt_t_pimpl::
  segmentID (const ::std::string& segmentID)
  {
    std::cout << "segmentID: " << segmentID << std::endl;
  }

  void RoadSegmentsAt_t_pimpl::
  post_RoadSegmentsAt_t ()
  {
  }

  // segment_t_pimpl
  //

  void segment_t_pimpl::
  pre ()
  {
	  rs = new sim_mob::RoadSegment();
  }

  void segment_t_pimpl::
  segmentID (const ::std::string& segmentID)
  {
	  this->rs->segmentID = segmentID;
  }

  void segment_t_pimpl::
  startingNode (const ::std::string& startingNode)
  {
	  if(this->rs->start)//nodes are loaded after links-segment-lane... so this is null
		  this->rs->start->setID(atoi(startingNode.c_str()));
  }

  void segment_t_pimpl::
  endingNode (const ::std::string& endingNode)
  {
	  if (this->rs->end)
	  this->rs->end->setID(atoi(endingNode.c_str()));
  }

  void segment_t_pimpl::
  maxSpeed (short maxSpeed)
  {
	  this->rs->maxSpeed = maxSpeed;
  }

  void segment_t_pimpl::
  Length (unsigned int Length)
  {
	  this->rs->length = Length;
  }

  void segment_t_pimpl::
  Width (unsigned int Width)
  {
	  this->rs->width = Width;
  }

  void segment_t_pimpl::
  Lanes (std::vector<sim_mob::Lane*> Lanes)
  {
	  this->rs->lanes =  Lanes;
  }

  void segment_t_pimpl::
  Obstacles ()
  {
  }

  void segment_t_pimpl::
  KurbLine (std::vector<sim_mob::Point2D> KurbLine)
  {
    // TODO
    //
  }

  sim_mob::RoadSegment* segment_t_pimpl::
  post_segment_t ()
  {
	  std::cout << "Returning A segment\n";
	  return rs;
  }

  // link_t_pimpl
  //

  void link_t_pimpl::
  pre ()
  {
	  link = new sim_mob::Link();
  }

  void link_t_pimpl::
  linkID (const ::std::string& linkID)
  {
	  link->linkID = linkID;
  }

  void link_t_pimpl::
  roadName (const ::std::string& roadName)
  {
	  link->roadName = roadName;
    std::cout << "roadName: " << roadName << std::endl;
  }

  void link_t_pimpl::
  StartingNode (const ::std::string& StartingNode)
  {
    std::cout << "StartingNode: " << StartingNode << std::endl;
  }

  void link_t_pimpl::
  EndingNode (const ::std::string& EndingNode)
  {
    std::cout << "EndingNode: " << EndingNode << std::endl;
  }

  void link_t_pimpl::
  Segments (std::pair<std::vector<sim_mob::RoadSegment*>,std::vector<sim_mob::RoadSegment*> > Segments)
  {

  }

  sim_mob::Link* link_t_pimpl::
  post_link_t ()
  {
    // TODO
    //
    // return ... ;
  }

  // separator_t_pimpl
  //

  void separator_t_pimpl::
  pre ()
  {
  }

  void separator_t_pimpl::
  separator_ID (unsigned short separator_ID)
  {
    std::cout << "separator_ID: " << separator_ID << std::endl;
  }

  void separator_t_pimpl::
  separator_value (bool separator_value)
  {
    std::cout << "separator_value: " << separator_value << std::endl;
  }

  void separator_t_pimpl::
  post_separator_t ()
  {
  }

  // separators_t_pimpl
  //

  void separators_t_pimpl::
  pre ()
  {
  }

  void separators_t_pimpl::
  Separator ()
  {
  }

  void separators_t_pimpl::
  post_separators_t ()
  {
  }

  // DomainIsland_t_pimpl
  //

  void DomainIsland_t_pimpl::
  pre ()
  {
  }

  void DomainIsland_t_pimpl::
  domainIsland_ID (unsigned short domainIsland_ID)
  {
    std::cout << "domainIsland_ID: " << domainIsland_ID << std::endl;
  }

  void DomainIsland_t_pimpl::
  domainIsland_value (bool domainIsland_value)
  {
    std::cout << "domainIsland_value: " << domainIsland_value << std::endl;
  }

  void DomainIsland_t_pimpl::
  post_DomainIsland_t ()
  {
  }

  // DomainIslands_t_pimpl
  //

  void DomainIslands_t_pimpl::
  pre ()
  {
  }

  void DomainIslands_t_pimpl::
  domainIslands ()
  {
  }

  void DomainIslands_t_pimpl::
  post_DomainIslands_t ()
  {
  }

  // offset_t_pimpl
  //

  void offset_t_pimpl::
  pre ()
  {
  }

  void offset_t_pimpl::
  offset_ID (unsigned short offset_ID)
  {
    std::cout << "offset_ID: " << offset_ID << std::endl;
  }

  void offset_t_pimpl::
  offset_value (unsigned int offset_value)
  {
    std::cout << "offset_value: " << offset_value << std::endl;
  }

  void offset_t_pimpl::
  post_offset_t ()
  {
  }

  // offsets_t_pimpl
  //

  void offsets_t_pimpl::
  pre ()
  {
  }

  void offsets_t_pimpl::
  offset ()
  {
  }

  void offsets_t_pimpl::
  post_offsets_t ()
  {
  }

  // ChunkLength_t_pimpl
  //

  void ChunkLength_t_pimpl::
  pre ()
  {
  }

  void ChunkLength_t_pimpl::
  chunklength_ID (unsigned short chunklength_ID)
  {
    std::cout << "chunklength_ID: " << chunklength_ID << std::endl;
  }

  void ChunkLength_t_pimpl::
  chunklength_value (unsigned int chunklength_value)
  {
    std::cout << "chunklength_value: " << chunklength_value << std::endl;
  }

  void ChunkLength_t_pimpl::
  post_ChunkLength_t ()
  {
  }

  // ChunkLengths_t_pimpl
  //

  void ChunkLengths_t_pimpl::
  pre ()
  {
  }

  void ChunkLengths_t_pimpl::
  chunklength ()
  {
  }

  void ChunkLengths_t_pimpl::
  post_ChunkLengths_t ()
  {
  }

  // LanesVector_t_pimpl
  //

  void LanesVector_t_pimpl::
  pre ()
  {
  }

  void LanesVector_t_pimpl::
  laneID (const ::std::string& laneID)
  {
    std::cout << "laneID: " << laneID << std::endl;
  }

  void LanesVector_t_pimpl::
  post_LanesVector_t ()
  {
  }

  // EntranceAngle_t_pimpl
  //

  void EntranceAngle_t_pimpl::
  pre ()
  {
  }

  void EntranceAngle_t_pimpl::
  entranceAngle_ID (unsigned short entranceAngle_ID)
  {
    std::cout << "entranceAngle_ID: " << entranceAngle_ID << std::endl;
  }

  void EntranceAngle_t_pimpl::
  entranceAngle_value (unsigned int entranceAngle_value)
  {
    std::cout << "entranceAngle_value: " << entranceAngle_value << std::endl;
  }

  void EntranceAngle_t_pimpl::
  post_EntranceAngle_t ()
  {
  }

  // EntranceAngles_t_pimpl
  //

  void EntranceAngles_t_pimpl::
  pre ()
  {
  }

  void EntranceAngles_t_pimpl::
  entranceAngle ()
  {
  }

  void EntranceAngles_t_pimpl::
  post_EntranceAngles_t ()
  {
  }

  // UniNode_t_pimpl
  //

  void UniNode_t_pimpl::
  pre ()
  {
  }

  void UniNode_t_pimpl::
  nodeID (const ::std::string& nodeID)
  {
    std::cout << "nodeID: " << nodeID << std::endl;
  }

  void UniNode_t_pimpl::
  location (sim_mob::Point2D location)
  {
    // TODO
    //
  }

  void UniNode_t_pimpl::
  Connectors ()
  {
  }

  sim_mob::UniNode* UniNode_t_pimpl::
  post_UniNode_t ()
  {
    // TODO
    //
    // return ... ;
  }

  // roundabout_t_pimpl
  //

  void roundabout_t_pimpl::
  pre ()
  {
  }

  void roundabout_t_pimpl::
  nodeID (const ::std::string& nodeID)
  {
    std::cout << "nodeID: " << nodeID << std::endl;
  }

  void roundabout_t_pimpl::
  location (sim_mob::Point2D location)
  {
    // TODO
    //
  }

  void roundabout_t_pimpl::
  roadSegmentsAt ()
  {
  }

  void roundabout_t_pimpl::
  Connectors ()
  {
  }

  void roundabout_t_pimpl::
  ChunkLengths ()
  {
  }

  void roundabout_t_pimpl::
  Offsets ()
  {
  }

  void roundabout_t_pimpl::
  Separators ()
  {
  }

  void roundabout_t_pimpl::
  addDominantLane ()
  {
  }

  void roundabout_t_pimpl::
  roundaboutDominantIslands (float roundaboutDominantIslands)
  {
    std::cout << "roundaboutDominantIslands: " << roundaboutDominantIslands << std::endl;
  }

  void roundabout_t_pimpl::
  roundaboutNumberOfLanes (int roundaboutNumberOfLanes)
  {
    std::cout << "roundaboutNumberOfLanes: " << roundaboutNumberOfLanes << std::endl;
  }

  void roundabout_t_pimpl::
  entranceAngles ()
  {
  }

  void roundabout_t_pimpl::
  obstacles (std::vector<sim_mob::RoadItem> obstacles)
  {
    // TODO
    //
  }

  sim_mob::MultiNode* roundabout_t_pimpl::
  post_roundabout_t ()
  {
    // TODO
    //
    // return ... ;
  }

  // intersection_t_pimpl
  //

  void intersection_t_pimpl::
  pre ()
  {
  }

  void intersection_t_pimpl::
  nodeID (const ::std::string& nodeID)
  {
    std::cout << "nodeID: " << nodeID << std::endl;
  }

  void intersection_t_pimpl::
  location (sim_mob::Point2D location)
  {
    // TODO
    //
  }

  void intersection_t_pimpl::
  roadSegmentsAt ()
  {
  }

  void intersection_t_pimpl::
  Connectors ()
  {
  }

  void intersection_t_pimpl::
  ChunkLengths ()
  {
  }

  void intersection_t_pimpl::
  Offsets ()
  {
  }

  void intersection_t_pimpl::
  Separators ()
  {
  }

  void intersection_t_pimpl::
  additionalDominantLanes ()
  {
  }

  void intersection_t_pimpl::
  additionalSubdominantLanes ()
  {
  }

  void intersection_t_pimpl::
  domainIslands ()
  {
  }

  void intersection_t_pimpl::
  obstacles (std::vector<sim_mob::RoadItem> obstacles)
  {
    // TODO
    //
  }

  sim_mob::MultiNode* intersection_t_pimpl::
  post_intersection_t ()
  {
    // TODO
    //
    // return ... ;
  }

  // RoadItem_No_Attr_t_pimpl
  //

  void RoadItem_No_Attr_t_pimpl::
  pre ()
  {
  }

  void RoadItem_No_Attr_t_pimpl::
  start (sim_mob::Point2D start)
  {
    // TODO
    //
  }

  void RoadItem_No_Attr_t_pimpl::
  end (sim_mob::Point2D end)
  {
    // TODO
    //
  }

  void RoadItem_No_Attr_t_pimpl::
  post_RoadItem_No_Attr_t ()
  {
  }

  // RoadItem_t_pimpl
  //

  void RoadItem_t_pimpl::
  pre ()
  {
  }

  void RoadItem_t_pimpl::
  Offset (unsigned short Offset)
  {
    std::cout << "Offset: " << Offset << std::endl;
  }

  sim_mob::RoadItem* RoadItem_t_pimpl::
  post_RoadItem_t ()
  {
    post_RoadItem_No_Attr_t ();
  }

  // BusStop_t_pimpl
  //

  void BusStop_t_pimpl::
  pre ()
  {
  }

  void BusStop_t_pimpl::
  busStopID (const ::std::string& busStopID)
  {
    std::cout << "busStopID: " << busStopID << std::endl;
  }

  void BusStop_t_pimpl::
  lane_location (const ::std::string& lane_location)
  {
    std::cout << "lane_location: " << lane_location << std::endl;
  }

  void BusStop_t_pimpl::
  is_Terminal (bool is_Terminal)
  {
    std::cout << "is_Terminal: " << is_Terminal << std::endl;
  }

  void BusStop_t_pimpl::
  is_Bay (bool is_Bay)
  {
    std::cout << "is_Bay: " << is_Bay << std::endl;
  }

  void BusStop_t_pimpl::
  has_shelter (bool has_shelter)
  {
    std::cout << "has_shelter: " << has_shelter << std::endl;
  }

  void BusStop_t_pimpl::
  busCapacityAsLength (unsigned int busCapacityAsLength)
  {
    std::cout << "busCapacityAsLength: " << busCapacityAsLength << std::endl;
  }

  void BusStop_t_pimpl::
  post_BusStop_t ()
  {
    sim_mob::RoadItem* v (post_RoadItem_t ());

    // TODO
    //
  }

  // ERP_Gantry_t_pimpl
  //

  void ERP_Gantry_t_pimpl::
  pre ()
  {
  }

  void ERP_Gantry_t_pimpl::
  ERP_GantryID (const ::std::string& ERP_GantryID)
  {
    std::cout << "ERP_GantryID: " << ERP_GantryID << std::endl;
  }

  void ERP_Gantry_t_pimpl::
  post_ERP_Gantry_t ()
  {
    sim_mob::RoadItem* v (post_RoadItem_t ());

    // TODO
    //
  }

  // FormType_pimpl
  //

  void FormType_pimpl::
  pre ()
  {
  }

  void FormType_pimpl::
  TextBox (int TextBox)
  {
    std::cout << "TextBox: " << TextBox << std::endl;
  }

  void FormType_pimpl::
  TextArea (int TextArea)
  {
    std::cout << "TextArea: " << TextArea << std::endl;
  }

  void FormType_pimpl::
  Header (int Header)
  {
    std::cout << "Header: " << Header << std::endl;
  }

  void FormType_pimpl::
  post_FormType ()
  {
  }

  // PointPair_t_pimpl
  //

  void PointPair_t_pimpl::
  pre ()
  {
  }

  void PointPair_t_pimpl::
  first (sim_mob::Point2D first)
  {
    // TODO
    //
  }

  void PointPair_t_pimpl::
  second (sim_mob::Point2D second)
  {
    // TODO
    //
  }

  void PointPair_t_pimpl::
  post_PointPair_t ()
  {
  }

  // crossing_t_pimpl
  //

  void crossing_t_pimpl::
  pre ()
  {
  }

  void crossing_t_pimpl::
  crossingID (const ::std::string& crossingID)
  {
    std::cout << "crossingID: " << crossingID << std::endl;
  }

  void crossing_t_pimpl::
  nearLine ()
  {
  }

  void crossing_t_pimpl::
  farLine ()
  {
  }

  sim_mob::Crossing* crossing_t_pimpl::
  post_crossing_t ()
  {
    sim_mob::RoadItem* v (post_RoadItem_t ());

    // TODO
    //
    // return ... ;
  }

  // RoadBump_t_pimpl
  //

  void RoadBump_t_pimpl::
  pre ()
  {
  }

  void RoadBump_t_pimpl::
  roadBumpID (const ::std::string& roadBumpID)
  {
    std::cout << "roadBumpID: " << roadBumpID << std::endl;
  }

  void RoadBump_t_pimpl::
  segmentID (const ::std::string& segmentID)
  {
    std::cout << "segmentID: " << segmentID << std::endl;
  }

  void RoadBump_t_pimpl::
  post_RoadBump_t ()
  {
    sim_mob::RoadItem* v (post_RoadItem_t ());

    // TODO
    //
  }

  // RoadNetwork_t_pimpl
  //

  void RoadNetwork_t_pimpl::
  pre ()
  {
  }

  void RoadNetwork_t_pimpl::
  Links (std::vector<sim_mob::Link*> Links)
  {
    // TODO
    //
  }

  void RoadNetwork_t_pimpl::
  Nodes ()
  {
  }

  void RoadNetwork_t_pimpl::
  post_RoadNetwork_t ()
  {
  }

  // RoadItems_t_pimpl
  //

  void RoadItems_t_pimpl::
  pre ()
  {
  }

  void RoadItems_t_pimpl::
  BusStop ()
  {
  }

  void RoadItems_t_pimpl::
  ERP_Gantry ()
  {
  }

  void RoadItems_t_pimpl::
  Crossing (sim_mob::Crossing* Crossing)
  {
    // TODO
    //
  }

  void RoadItems_t_pimpl::
  RoadBump ()
  {
  }

  void RoadItems_t_pimpl::
  post_RoadItems_t ()
  {
  }

  // GeoSpatial_t_pimpl
  //

  void GeoSpatial_t_pimpl::
  pre ()
  {
  }

  void GeoSpatial_t_pimpl::
  RoadNetwork ()
  {
  }

  void GeoSpatial_t_pimpl::
  post_GeoSpatial_t ()
  {
  }

  // SimMobility_t_pimpl
  //

  void SimMobility_t_pimpl::
  pre ()
  {
  }

  void SimMobility_t_pimpl::
  GeoSpatial ()
  {
  }

  void SimMobility_t_pimpl::
  post_SimMobility_t ()
  {
  }

  // Lanes_pimpl
  //

  void Lanes_pimpl::
  pre ()
  {
  }

  void Lanes_pimpl::
  Lane (sim_mob::Lane* Lane)
  {
	  this->lanes.push_back(Lane);
  }

  std::vector<sim_mob::Lane*> Lanes_pimpl::
  post_Lanes ()
  {
	  std::cout<< "Returning Lanes\n";
	  return this->lanes;
  }

  // Segments_pimpl
  //

  void Segments_pimpl::
  pre ()
  {
  }

  void Segments_pimpl::
  FWDSegments (std::vector<sim_mob::RoadSegment*> FWDSegments)
  {
	  fwd = FWDSegments;
  }

  void Segments_pimpl::
  BKDSegments (std::vector<sim_mob::RoadSegment*> BKDSegments)
  {
	  bck = BKDSegments;
  }

  std::pair<std::vector<sim_mob::RoadSegment*>,std::vector<sim_mob::RoadSegment*> > Segments_pimpl::
  post_Segments ()
  {
	  return (std::make_pair(fwd,bck));
  }

  // Links_pimpl
  //

  void Links_pimpl::
  pre ()
  {
  }

  void Links_pimpl::
  Link (sim_mob::Link* Link)
  {
    // TODO
    //
  }

  std::vector<sim_mob::Link*> Links_pimpl::
  post_Links ()
  {
    // TODO
    //
    // return ... ;
  }

  // Nodes_pimpl
  //

  void Nodes_pimpl::
  pre ()
  {
  }

  void Nodes_pimpl::
  UniNodes (std::set<sim_mob::UniNode*> UniNodes)
  {
    // TODO
    //
  }

  void Nodes_pimpl::
  Intersections (std::vector<sim_mob::MultiNode*> Intersections)
  {
    // TODO
    //
  }

  void Nodes_pimpl::
  roundabouts (std::vector<sim_mob::MultiNode*> roundabouts)
  {
    // TODO
    //
  }

  void Nodes_pimpl::
  post_Nodes ()
  {
  }

  // UniNodes_pimpl
  //

  void UniNodes_pimpl::
  pre ()
  {
  }

  void UniNodes_pimpl::
  UniNode (sim_mob::UniNode* UniNode)
  {
    // TODO
    //
  }

  std::set<sim_mob::UniNode*> UniNodes_pimpl::
  post_UniNodes ()
  {
    // TODO
    //
    // return ... ;
  }

  // Intersections_pimpl
  //

  void Intersections_pimpl::
  pre ()
  {
  }

  void Intersections_pimpl::
  Intersection (sim_mob::MultiNode* Intersection)
  {
    // TODO
    //
  }

  std::vector<sim_mob::MultiNode*> Intersections_pimpl::
  post_Intersections ()
  {
    // TODO
    //
    // return ... ;
  }

  // roundabouts_pimpl
  //

  void roundabouts_pimpl::
  pre ()
  {
  }

  void roundabouts_pimpl::
  roundabout (sim_mob::MultiNode* roundabout)
  {
    // TODO
    //
  }

  std::vector<sim_mob::MultiNode*> roundabouts_pimpl::
  post_roundabouts ()
  {
    // TODO
    //
    // return ... ;
  }
}

