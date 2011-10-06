/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Loader.hpp"

#include <cmath>
#include <algorithm>
#include <stdexcept>

//NOTE: Ubuntu is pretty bad about where it puts the SOCI headers.
//      "soci-postgresql.h" is supposed to be in "$INC/soci", but Ubuntu puts it in
//      "$INC/soci/postgresql". For now, I'm just referencing it manually, but
//      we might want to use something like pkg-config to manage header file directories
//      eventually.
#include "soci/soci.h"
#include "soci-postgresql.h"

#include "../Point2D.hpp"
#include "../Node.hpp"
#include "../UniNode.hpp"
#include "../MultiNode.hpp"
#include "../Intersection.hpp"
#include "../Link.hpp"
#include "../RoadSegment.hpp"
#include "../LaneConnector.hpp"
#include "../RoadNetwork.hpp"
#include "../Crossing.hpp"
#include "../Lane.hpp"

#include "../../util/DynamicVector.hpp"
#include "../../util/OutputUtil.hpp"

#include "Node.hpp"
#include "Section.hpp"
#include "Crossing.hpp"
#include "Turning.hpp"
#include "Polyline.hpp"
#include "SOCI_Converters.hpp"


using namespace sim_mob::aimsun;
using sim_mob::DynamicVector;
using std::vector;
using std::string;
using std::set;
using std::map;
using std::pair;
using std::multimap;


namespace {



//Sorting function for polylines
bool polyline_sorter (const Polyline* const p1, const Polyline* const p2)
{
	return p1->distanceFromSrc < p2->distanceFromSrc;
}



int minID(const vector<double>& vals)
{
	int res = -1;
	for (size_t i=0; i<vals.size(); i++) {
		if (res==-1 || (vals[i]<vals[res])) {
			res = i;
		}
	}
	return res;
}


void LoadNodes(soci::session& sql, const std::string& storedProc, map<int, Node>& nodelist)
{
	//Our SQL statement
	soci::rowset<Node> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	nodelist.clear();
	for (soci::rowset<Node>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		if (nodelist.count(it->id)>0) {
			throw std::runtime_error("Duplicate AIMSUN node.");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		nodelist[it->id] = *it;
	}
}


void LoadSections(soci::session& sql, const std::string& storedProc, map<int, Section>& sectionlist, map<int, Node>& nodelist)
{
	//Our SQL statement
	soci::rowset<Section> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	sectionlist.clear();
	for (soci::rowset<Section>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		if(nodelist.count(it->TMP_FromNodeID)==0 || nodelist.count(it->TMP_ToNodeID)==0) {
			std::cout <<"From node: " <<it->TMP_FromNodeID  <<"  " <<nodelist.count(it->TMP_FromNodeID) <<"\n";
			std::cout <<"To node: " <<it->TMP_ToNodeID  <<"  " <<nodelist.count(it->TMP_ToNodeID) <<"\n";
			throw std::runtime_error("Invalid From or To node.");
		}

		//Convert meters to cm
		it->length *= 100;

		//Note: Make sure not to resize the Node map after referencing its elements.
		it->fromNode = &nodelist[it->TMP_FromNodeID];
		it->toNode = &nodelist[it->TMP_ToNodeID];
		sectionlist[it->id] = *it;
	}
}


void LoadCrossings(soci::session& sql, const std::string& storedProc, vector<Crossing>& crossings, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Crossing> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	crossings.clear();
	for (soci::rowset<Crossing>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check sections
		if(sectionlist.count(it->TMP_AtSectionID)==0) {
			throw std::runtime_error("Crossing at Invalid Section");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Note: Make sure not to resize the Section vector after referencing its elements.
		it->atSection = &sectionlist[it->TMP_AtSectionID];
		crossings.push_back(*it);
	}
}

void LoadLanes(soci::session& sql, const std::string& storedProc, vector<Lane>& lanes, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Lane> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	lanes.clear();
	for (soci::rowset<Lane>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check sections
		if(sectionlist.count(it->TMP_AtSectionID)==0) {
			throw std::runtime_error("Crossing at Invalid Section");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Exclude "crossing" types
		if (it->laneType=="J" || it->laneType=="A4") {
			continue;
		}

		//Exclude lane markings which are not relevant to actual lane geometry
		if (it->laneType=="R" || it->laneType=="M" || it->laneType=="D" || it->laneType=="N"
			|| it->laneType=="Q" || it->laneType=="T" || it->laneType=="G" || it->laneType=="O"
			|| it->laneType=="A1" || it->laneType=="A3" || it->laneType=="L" || it->laneType=="H"
			|| it->laneType=="\\N"
			) {
			continue;
		}

		//Note: Make sure not to resize the Section vector after referencing its elements.
		it->atSection = &sectionlist[it->TMP_AtSectionID];
		lanes.push_back(*it);
	}
}


void LoadTurnings(soci::session& sql, const std::string& storedProc, map<int, Turning>& turninglist, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Turning> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	vector<int> skippedTurningIDs;
	turninglist.clear();
	for (soci::rowset<Turning>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		bool fromMissing = sectionlist.count(it->TMP_FromSection)==0;
		bool toMissing = sectionlist.count(it->TMP_ToSection)==0;
		if(fromMissing || toMissing) {
			skippedTurningIDs.push_back(it->id);
			continue;
		}

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->fromSection = &sectionlist[it->TMP_FromSection];
		it->toSection = &sectionlist[it->TMP_ToSection];
		turninglist[it->id] = *it;
	}

	//Print skipped turnings all at once.
	sim_mob::PrintArray(skippedTurningIDs, "Turnings skipped: ", "[", "]", ", ", 4);
}

void LoadPolylines(soci::session& sql, const std::string& storedProc, multimap<int, Polyline>& polylinelist, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Polyline> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	polylinelist.clear();
	for (soci::rowset<Polyline>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		if(sectionlist.count(it->TMP_SectionId)==0) {
			throw std::runtime_error("Invalid polyline section reference.");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->section = &sectionlist[it->TMP_SectionId];
		polylinelist.insert(std::make_pair(it->section->id, *it));
		//polylinelist[it->id] = *it;
	}
}



void LoadBasicAimsunObjects(const string& connectionStr, map<string, string>& storedProcs, map<int, Node>& nodes, map<int, Section>& sections, vector<Crossing>& crossings, vector<Lane>& lanes, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//Connect
	//Connection string will look something like this:
	//"host=localhost port=5432 dbname=SimMobility_DB user=postgres password=XXXXX"
	soci::session sql(soci::postgresql, connectionStr);

	//Load all nodes
	LoadNodes(sql, storedProcs["node"], nodes);

	//Load all sections
	LoadSections(sql, storedProcs["section"], sections, nodes);

	//Load all crossings
	LoadCrossings(sql, storedProcs["crossing"], crossings, sections);

	//Load all lanes
	LoadLanes(sql, storedProcs["lane"], lanes, sections);

	//Load all turnings
	LoadTurnings(sql, storedProcs["turning"], turnings, sections);

	//Load all polylines
	LoadPolylines(sql, storedProcs["polyline"], polylines, sections);
}



/**
 * Given a set of points that make up a lane line, sort them as follows:
 * 1) Pick the "first" point. This one is the closest to either Node1 or Node2 in the nodes pair.
 * 2) Set "last" = "first"
 * 2) Continue picking the point which is closest to "last", adding it, then setting "last" equal to that point.
 */
void SortLaneLine(vector<Lane*>& laneLine, std::pair<Node*, Node*> nodes)
{
	//Quality control
	size_t oldSize = laneLine.size();

	//Pick the first point.
	double currDist = 0.0;
	bool flipLater = false;
	vector<Lane*>::iterator currLane = laneLine.end();
	for (vector<Lane*>::iterator it=laneLine.begin(); it!=laneLine.end(); it++) {
		double distFwd = distLaneNode(*it, nodes.first);
		double distRev = distLaneNode(*it, nodes.second);
		double newDist = std::min(distFwd, distRev);
		if (currLane==laneLine.end() || newDist<currDist) {
			currDist = newDist;
			currLane = it;
			flipLater = distRev<distFwd;
		}
	}

	//Continue adding points and selecting candidates
	vector<Lane*> res;
	while (currLane!=laneLine.end()) {
		//Add it, remove it, null it.
		res.push_back(*currLane);
		laneLine.erase(currLane);
		currLane = laneLine.end();

		//Pick the next lane
		for (vector<Lane*>::iterator it=laneLine.begin(); it!=laneLine.end(); it++) {
			double newDist = distLaneLane(res.back(), *it);
			if (currLane==laneLine.end() || newDist<currDist) {
				currDist = newDist;
				currLane = it;

				//TODO: We might want to see what happens here if newDist is zero.
				//      In the previous tests I ran, there is only ONE pair of points with the exact
				//      same coordinates, but about 5 other pairs are very very close together.
			}
		}
	}

	//Check
	laneLine.clear();
	if (oldSize != res.size()) {
		std::cout <<"ERROR: Couldn't sort Lanes array, zeroing out.\n";
	}


	//Finally, if the "end" is closer to the start node than the "start", reverse the vector as you insert it
	if (flipLater) {
		for (vector<Lane*>::reverse_iterator it=res.rbegin(); it!=res.rend(); it++) {
			laneLine.push_back(*it);
		}
	} else {
		laneLine.insert(laneLine.begin(), res.begin(), res.end());
	}
}




//Compute the distance from the source node of the polyline to a
// point on the line from the source to the destination nodes which
// is normal to the Poly-point.
void ComputePolypointDistance(Polyline& pt)
{
	//Our method is (fairly) simple.
	//First, compute the distance from the point to the polyline at a perpendicular angle.
	double dx2x1 = pt.section->toNode->xPos - pt.section->fromNode->xPos;
	double dy2y1 = pt.section->toNode->yPos - pt.section->fromNode->yPos;
	double dx1x0 = pt.section->fromNode->xPos - pt.xPos;
	double dy1y0 = pt.section->fromNode->yPos - pt.yPos;
	double numerator = dx2x1*dy1y0 - dx1x0*dy2y1;
	double denominator = sqrt(dx2x1*dx2x1 + dy2y1*dy2y1);
	double perpenDist = numerator/denominator;
	if (perpenDist<0.0) {
		//We simplify all the quadratic math to just a sign change, since
		//   it's known that this polypoint has a positive distance to the line.
		perpenDist *= -1;
	}

	//Second, compute the distance from the source point to the polypoint
	double realDist = sqrt(dx1x0*dx1x0 + dy1y0*dy1y0);

	//Finally, apply the Pythagorean theorum
	pt.distanceFromSrc = sqrt(realDist*realDist - perpenDist*perpenDist);

	//NOTE: There simplest method would be to just take the x-component of the vector
	//      from pt.x/y to pt.section.fromNode.x/y, but you'd have to factor in
	//      the fact that this vector is rotated with respect to pt.section.from->pt.section.to.
	//      I can't remember enough vector math to handle this, but if anyone wants to
	//      replace it the vector version would certainly be faster. ~Seth
}



void DecorateAndTranslateObjects(map<int, Node>& nodes, map<int, Section>& sections, vector<Crossing>& crossings, vector<Lane>& lanes, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//Step 1: Tag all Nodes with the Sections that meet there.
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		it->second.fromNode->sectionsAtNode.push_back(&(it->second));
		it->second.toNode->sectionsAtNode.push_back(&(it->second));
	}


	//Step 2: Tag all Nodes that might be "UniNodes". These fit the following criteria:
	//        1) In ALL sections that meet at this node, there are only two distinct nodes.
	//        2) Each of these distinct nodes has exactly ONE Segment leading "from->to" and one leading "to->from".
	//           This should take bi-directional Segments into account.
	//        3) All Segments share the same Road Name
	//        4) Optionally, there can be a single link in ONE direction, representing a one-way road.
	vector<int> nodeMismatchIDs;
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		Node* n = &it->second;
		n->candidateForSegmentNode = true; //Conditional pass

		//Perform both checks at the same time.
		pair<Node*, Node*> others(nullptr, nullptr);
		pair<unsigned int, unsigned int> flags(0, 0);  //1="from->to", 2="to->from"
		string expectedName;
		for (vector<Section*>::iterator it=n->sectionsAtNode.begin(); it!=n->sectionsAtNode.end(); it++) {
			//Get "other" node
			Node* otherNode = ((*it)->fromNode!=n) ? (*it)->fromNode : (*it)->toNode;

			//Manage property one.
			unsigned int* flagPtr;
			if (!others.first || others.first==otherNode) {
				others.first = otherNode;
				flagPtr = &flags.first;
			} else if (!others.second || others.second==otherNode) {
				others.second = otherNode;
				flagPtr = &flags.second;
			} else {
				n->candidateForSegmentNode = false; //Fail
				break;
			}

			//Manage property two.
			unsigned int toFlag = ((*it)->toNode==n) ? 1 : 2;
			if (((*flagPtr)&toFlag)==0) {
				*flagPtr = (*flagPtr) | toFlag;
			} else {
				n->candidateForSegmentNode = false; //Fail
				break;
			}

			//Manage property three.
			if (expectedName.empty()) {
				expectedName = (*it)->roadName;
			} else if (expectedName != (*it)->roadName) {
				n->candidateForSegmentNode = false; //Fail
				break;
			}
		}

		//One final check
		if (n->candidateForSegmentNode) {
			bool flagMatch =   (flags.first==3 && flags.second==3)  //Bidirectional
							|| (flags.first==1 && flags.second==2)  //One-way
							|| (flags.first==2 && flags.second==1); //One-way

			n->candidateForSegmentNode = others.first && others.second && flagMatch;
		}

		//Generate warnings if this value doesn't match the expected "is intersection" value.
		//This is usually a result of a network being cropped.
		if (n->candidateForSegmentNode == n->isIntersection) {
			nodeMismatchIDs.push_back(n->id);
		}
	}

	//Print all node mismatches at once
	sim_mob::PrintArray(nodeMismatchIDs, "UniNode/Intersection mismatches: ", "[", "]", ", ", 4);

	//Step 3: Tag all Sections with Turnings that apply to that Section
	for (map<int,Turning>::iterator it=turnings.begin(); it!=turnings.end(); it++) {
		it->second.fromSection->connectedTurnings.push_back(&(it->second));
		it->second.toSection->connectedTurnings.push_back(&(it->second));
	}

	//Step 4: Add polyline entries to Sections. As you do this, compute their distance
	//        from the origin ("from" node)
	for (map<int,Polyline>::iterator it=polylines.begin(); it!=polylines.end(); it++) {
		it->second.section->polylineEntries.push_back(&(it->second));
		ComputePolypointDistance(it->second);
	}

	//Step 4.5: Add all Lanes to their respective Sections. Then sort each line segment
	for (vector<Lane>::iterator it=lanes.begin(); it!=lanes.end(); it++) {
		it->atSection->laneLinesAtNode[it->laneID].push_back(&(*it));
	}
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		for (map<int, vector<Lane*> >::iterator laneIt=it->second.laneLinesAtNode.begin(); laneIt!=it->second.laneLinesAtNode.end(); laneIt++) {
			SortLaneLine(laneIt->second, std::make_pair(it->second.fromNode, it->second.toNode));
		}
	}


	//Steps 5,6: Request the CrossingsLoader to tag some Crossing-related data.
	CrossingLoader::DecorateCrossings(nodes, crossings);

}


//Helpers for Lane construction
struct LaneSingleLine { //Used to represent a set of Lanes by id.
	vector<Lane*> points;
	LaneSingleLine() {}
	LaneSingleLine(const vector<Lane*>& mypoints) {
		points.insert(points.begin(), mypoints.begin(), mypoints.end());
	}

	double ComputeAngle(Lane* start, Lane* end) {
		double dx = end->xPos - start->xPos;
		double dy = end->yPos - start->yPos;
		return atan2(dy, dx);
	}

	void computeAndSaveAngle() {
		double maxLen = 0.0;
		Lane* pastLane = nullptr;
		for (vector<Lane*>::iterator currLane=points.begin(); currLane!=points.end(); currLane++) {
			if (pastLane) {
				double currLen = distLaneLane(pastLane, *currLane);
				if (currLen > maxLen) {
					maxLen = currLen;
					angle = ComputeAngle(pastLane, *currLane);
				}
			}
			//Save
			pastLane = *currLane;
		}
	}

	//For sorting, later
	double angle;
	double minDist;
};
struct LinkHelperStruct {
	Node* start;
	Node* end;
	set<Section*> sections;
	LinkHelperStruct() : start(nullptr), end(nullptr) {}
};
map<sim_mob::Link*, LinkHelperStruct> buildLinkHelperStruct(map<int, Node>& nodes, map<int, Section>& sections)
{
	map<sim_mob::Link*, LinkHelperStruct> res;
	for (map<int, Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		//Always add the section
		sim_mob::Link* parent = it->second.generatedSegment->getLink();
		res[parent].sections.insert(&(it->second));

		//Conditionally add the start/end
		if (!res[parent].start) {
			if (it->second.fromNode->generatedNode == parent->getStart()) {
				res[parent].start = it->second.fromNode;
			} else if (it->second.toNode->generatedNode == parent->getStart()) {
				res[parent].start = it->second.toNode;
			}
		}
		if (!res[parent].end) {
			if (it->second.fromNode->generatedNode == parent->getEnd()) {
				res[parent].end = it->second.fromNode;
			} else if (it->second.toNode->generatedNode == parent->getEnd()) {
				res[parent].end = it->second.toNode;
			}
		}
	}

	return res;
}



void SaveSimMobilityNetwork(sim_mob::RoadNetwork& res, map<int, Node>& nodes, map<int, Section>& sections, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//First, Nodes. These match cleanly to the Sim Mobility data structures
	std::cout <<"Warning: Units are not considered when converting AIMSUN data.\n";
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		sim_mob::aimsun::Loader::ProcessGeneralNode(res, it->second);
	}

	//Next, Links and RoadSegments. See comments for our approach.
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		if (!it->second.hasBeenSaved) {  //Workaround...
			sim_mob::aimsun::Loader::ProcessSection(res, it->second);
		}
	}
	//Scan the vector to see if any skipped Sections were not filled in later.
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		if (!it->second.hasBeenSaved) {
			throw std::runtime_error("Section was skipped.");
		}
	}

	//Next, SegmentNodes (UniNodes), which are only partially initialized in the general case.
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		if (it->second.candidateForSegmentNode) {
			sim_mob::aimsun::Loader::ProcessUniNode(res, it->second);
		}
	}

	//Next, Turnings. These generally match up.
	std::cout <<"Warning: Lanes-Left-of-Divider incorrect when converting AIMSUN data.\n";
	for (map<int,Turning>::iterator it=turnings.begin(); it!=turnings.end(); it++) {
		sim_mob::aimsun::Loader::ProcessTurning(res, it->second);
	}

	//Next, save the Polylines. This is best done at the Section level
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		sim_mob::aimsun::Loader::ProcessSectionPolylines(res, it->second);
	}

	//Finalize our MultiNodes' circular arrays
	for (vector<sim_mob::MultiNode*>::const_iterator it=res.getNodes().begin(); it!=res.getNodes().end(); it++) {
		sim_mob::MultiNode::BuildClockwiseLinks(res, *it);
	}

	//Prune Crossings and convert to the "near" and "far" syntax of Sim Mobility. Also give it a "position", defined
	//   as halfway between the midpoints of the near/far lines, and then assign it as an Obstacle to both the incoming and
	//   outgoing RoadSegment that it crosses.
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		for (map<Node*, std::vector<int> >::iterator i2=it->second.crossingLaneIdsByOutgoingNode.begin(); i2!=it->second.crossingLaneIdsByOutgoingNode.end(); i2++) {
			CrossingLoader::GenerateACrossing(res, it->second, *i2->first, i2->second);
		}
	}

	//Prune lanes and figure out where the median is.
	// TODO: This should eventually allow other lanes to be designated too.
	map<sim_mob::Link*, LinkHelperStruct> lhs = buildLinkHelperStruct(nodes, sections);
	for (map<sim_mob::Link*, LinkHelperStruct>::iterator it=lhs.begin(); it!=lhs.end(); it++) {
		sim_mob::aimsun::Loader::GenerateLinkLaneZero(res, it->second.start, it->second.end, it->second.sections);
	}
}


double getClosestPoint(const vector<Lane*>& candidates, double xPos, double yPos)
{
	//Make searching slightly easier.
	Lane origin;
	origin.xPos = xPos;
	origin.yPos = yPos;

	//Search
	pair<double, Lane*> res(0.0, nullptr);
	for (vector<Lane*>::const_iterator it=candidates.begin(); it!=candidates.end(); it++) {
		double currDist = distLaneLane(&origin, *it);
		if (!res.second || currDist<res.first) {
			res.first = currDist;
			res.second = *it;
		}
	}

	return res.first;
}



//Remove candidates until either maxSize remain, or their total difference in angles is maxAngleDelta
void TrimCandidateList(vector<LaneSingleLine>& candidates, size_t maxSize, double maxAngleDelta)
{
	//Need to do anything?
	if (candidates.size()<=maxSize || candidates.empty()) {
		return;
	}

	//Simple strategy: Compute the angle for each of these long segments.
	for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
		it->computeAndSaveAngle();
	}

	//Normalize the angles
	double minVal = candidates.front().angle;
	double maxVal = candidates.front().angle;
	for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
		if (it->angle < minVal) {
			minVal = it->angle;
		}
		if (it->angle > maxVal) {
			maxVal = it->angle;
		}
	}
	for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
		it->angle = (it->angle-minVal) / (maxVal-minVal);
	}


	//Now find the set of size MaxSize with the minimum max-distance-between-any-2-points
	//The set of candidates is always quite small, so any brute-force algorithm will work.
	//while (candidates.size()>maxSize) {
	for (;;) {
		//Step one: For each LaneLine, find the LaneLine with the closest angle.
		for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
			it->minDist = 10; //Distance will never be more than 2PI
			for (vector<LaneSingleLine>::iterator other=candidates.begin(); other!=candidates.end(); other++) {
				//Skip self.
				if (&(*it) == &(*other)) {
					continue;
				}
				double currDist = fabs(other->angle - it->angle);
				if (currDist < it->minDist) {
					it->minDist = currDist;
				}
			}
		}

		//Step two: Find the candidate with the greatest min distance and remove it.
		vector<LaneSingleLine>::iterator maxIt;
		double maxDist = -1;
		for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
			if (it->minDist > maxDist) {
				maxIt = it;
				maxDist = it->minDist;
			}
		}

		//Before we erase it, check if it is below our theta threshold
		double actualTheta = maxDist*(maxVal-minVal);
		if (actualTheta<=maxAngleDelta) {
			break;
		}
		candidates.erase(maxIt);

		//Step three: Check if we're done
		if (candidates.size()<=maxSize || candidates.empty()) {
			break;
		}
	}
}


bool PointIsLeftOfVector(double ax, double ay, double bx, double by, double cx, double cy)
{
	//Via cross-product
	return ((bx - ax)*(cy - ay) - (by - ay)*(cx - ax)) > 0;
}
bool PointIsLeftOfVector(const Node* vecStart, const Node* vecEnd, const Lane* point)
{
	return PointIsLeftOfVector(vecStart->xPos, vecStart->yPos, vecEnd->xPos, vecEnd->yPos, point->xPos, point->yPos);
}
bool PointIsLeftOfVector(const DynamicVector& vec, const Lane* point)
{
	return PointIsLeftOfVector(vec.getX(), vec.getY(), vec.getEndX(), vec.getEndY(), point->xPos, point->yPos);
}



Lane* GetX_EstPoint(Lane* from, const vector<Lane*>& points, int sign)
{
	pair<Lane*, double> res(nullptr, 0.0);
	for (vector<Lane*>::const_iterator it=points.begin(); it!=points.end(); it++) {
		double currDist = distLaneLane(from, *it);
		if (!res.first || currDist*sign < res.second*sign) {
			res.first = *it;
			res.second = currDist;
		}
	}
	return res.first;
}


Lane* GetNearestPoint(Lane* from, const vector<Lane*>& points)
{
	return GetX_EstPoint(from, points, 1);
}
Lane* GetFarthestPoint(Lane* from, const vector<Lane*>& points)
{
	return GetX_EstPoint(from, points, -1);
}


void OrganizePointsInDrivingDirection(bool drivesOnLHS, Node* start, Node* end, vector<Lane*>& points)
{
	//Step 1: retrieve the two endpoints
	pair<Lane*, Lane*> endpoints;
	endpoints.first = GetFarthestPoint(points[0], points);
	endpoints.second = GetFarthestPoint(endpoints.first, points);
	endpoints.first = GetFarthestPoint(endpoints.second, points);

	//Step 2: Figure out which of these is "left of" the line
	bool firstIsLeft = PointIsLeftOfVector(start, end, endpoints.first);

	//Step 3: Star from the first (or last, depending on drivesOnLHS and firstIsLeft) point to the result vector.
	vector<Lane*> res;
	Lane* curr = firstIsLeft==drivesOnLHS ? endpoints.first : endpoints.second;

	//Step 4: Continue adding points until the old vector is empty, then restore it
	while (!points.empty()) {
		//Add, remove
		res.push_back(curr);
		points.erase(std::find(points.begin(), points.end(), curr));

		//Get the next nearest point
		curr = GetNearestPoint(curr, points);
	}
	points.insert(points.begin(), res.begin(), res.end());
}


//Determine the median when we know there are two Sections here.
Lane DetermineNormalMedian(const vector<Lane*>& orderedPoints, Section* fwdSec, Section* revSec)
{
	//If we have exactly the right number of lanes...
	if ((int)orderedPoints.size() == fwdSec->numLanes + revSec->numLanes + 1) {
		//...then return the lane which both Sections consider the median.
		return *orderedPoints[fwdSec->numLanes];
	} else {
		//...otherwise, form a vector from the first point to the last point, scale it
		//   back by half, and take that as your point.
		DynamicVector halfway(orderedPoints.front()->xPos, orderedPoints.front()->yPos, orderedPoints.back()->xPos, orderedPoints.back()->yPos);
		double scaleFactor = halfway.getMagnitude() / 2.0;
		halfway.makeUnit();
		halfway.scaleVect(scaleFactor);
		halfway.translateVect();

		Lane res;
		res.xPos = halfway.getX();
		res.yPos = halfway.getY();
		return Lane(res);
	}
}


pair<Lane, Lane> ComputeMedianEndpoints(bool drivesOnLHS, Node* start, Node* end, const pair< vector<LaneSingleLine>, vector<LaneSingleLine> >& candidates, const pair< size_t, size_t >& maxCandidates, const std::set<Section*>& allSections)
{
	Lane startPoint;
	Lane endPoint;

	//Create our vectors of points
	vector<Lane*> originPoints;
	for (vector<LaneSingleLine>::const_iterator it=candidates.first.begin(); it!=candidates.first.end(); it++) {
		originPoints.push_back(it->points[0]);
	}
	vector<Lane*> endingPoints;
	for (vector<LaneSingleLine>::const_iterator it=candidates.second.begin(); it!=candidates.second.end(); it++) {
		endingPoints.push_back(it->points[it->points.size()-1]);
	}

	//Sort the candidate lists so that, standing at "start" and looking at "end",
	//  they run left-to-right (or right-to-left if we are driving on the right)
	OrganizePointsInDrivingDirection(drivesOnLHS, start, end, originPoints);
	OrganizePointsInDrivingDirection(drivesOnLHS, start, end, endingPoints);

	//Determine if this is a single-diretional link
	std::pair<Section*, Section*> fwdFirstLastSection(nullptr, nullptr);
	std::pair<Section*, Section*> revFirstLastSection(nullptr, nullptr);
	for (std::set<Section*>::const_iterator it=allSections.begin(); it!=allSections.end(); it++) {
		if ((*it)->fromNode->id==start->id) {
			fwdFirstLastSection.first = *it;
		}
		if ((*it)->fromNode->id==end->id) {
			revFirstLastSection.first = *it;
		}
		if ((*it)->toNode->id==start->id) {
			revFirstLastSection.second = *it;
		}
		 if ((*it)->toNode->id==end->id) {
			 fwdFirstLastSection.second = *it;
		}
	}
	if (!(fwdFirstLastSection.first && fwdFirstLastSection.second)) {
		throw std::runtime_error("Unexpected: Link has no forward path.");
	}

	//If this is a single directional Link...
	if (!(revFirstLastSection.first && revFirstLastSection.second)) {
		//...then the median is the last point in the driving direction (e.g., the median).
		// Note that Links always have a forward path, but may not have a reverse path.
		startPoint = *originPoints.back();
		endPoint = *endingPoints.back();
	} else {
		//...otherwise, we deal with each point separately.
		startPoint = DetermineNormalMedian(originPoints, fwdFirstLastSection.first, revFirstLastSection.second);
		endPoint = DetermineNormalMedian(endingPoints, revFirstLastSection.first, fwdFirstLastSection.second);
	}

	return std::make_pair(startPoint, endPoint);
}



bool LanesWithinBounds(const vector<Lane*>& lanes, const pair<DynamicVector, DynamicVector>& bounds)
{
	pair<DynamicVector, DynamicVector> bounds2;
	bounds2.first = DynamicVector(bounds.first.getX(), bounds.first.getY(), bounds.second.getX(), bounds.second.getY());
	bounds2.second = DynamicVector(bounds.first.getEndX(), bounds.first.getEndY(), bounds.second.getEndX(), bounds.second.getEndY());
	for (vector<Lane*>::const_iterator it=lanes.begin(); it!=lanes.end(); it++) {
		const Lane* ln = *it;

		//Check first pair of bounds
		if (!PointIsLeftOfVector(bounds.first, ln) || PointIsLeftOfVector(bounds.second, ln)) {
			return false;
		}

		//Check second pair of bounds
		if (PointIsLeftOfVector(bounds2.first, ln) || !PointIsLeftOfVector(bounds2.second, ln)) {
			return false;
		}
	}

	//All points are within the bounds.
	return true;
}



vector<LaneSingleLine> CalculateSectionGeneralAngleCandidateList(const pair<Section*, Section*>& currSectPair, double singleLaneWidth, double threshhold)
{
	//Create some helpers for our Section.
	//int numLanes = currSectPair.first->numLanes + (currSectPair.second?currSectPair.second->numLanes:0);
	//double laneWidth = totalWidth / numLanes;
	double bufferSz = 1.1;

	//Create a midline vector, scale it out by a certain amount so that we catch stray points.
	DynamicVector midLine(currSectPair.first->fromNode->xPos, currSectPair.first->fromNode->yPos, currSectPair.first->toNode->xPos, currSectPair.first->toNode->yPos);
	double totalMag = midLine.getMagnitude()*bufferSz;
	double hwDiff = (totalMag-midLine.getMagnitude())/2;
	midLine.makeUnit().flipMirror().scaleVect(hwDiff).translateVect();
	midLine.makeUnit().flipMirror().scaleVect(totalMag);

	//Now, create a bounding box for our Section
	// We first create two vectors pointing "down" from our fwd section to/past our "rev" section (if it exists). These are scaled by a small amount.
	std::pair<DynamicVector, DynamicVector> startEndEdges;
	startEndEdges.first = DynamicVector(midLine);
	startEndEdges.second = DynamicVector(midLine);
	startEndEdges.second.translateVect().flipMirror();
	for (size_t id=0; id<2; id++) {
		DynamicVector& currEdge = id==0?startEndEdges.first:startEndEdges.second;
		currEdge.makeUnit();
		currEdge.flipNormal(id==1);
		currEdge.scaleVect( (bufferSz/2)*(currSectPair.first->numLanes*singleLaneWidth) );
		currEdge.translateVect();
		currEdge.flipMirror();
		if (currSectPair.second) {
			totalMag = currEdge.getMagnitude() + (bufferSz/2)*(currSectPair.second->numLanes*singleLaneWidth);
			currEdge.makeUnit();
			currEdge.scaleVect(totalMag);
		}
	}

	//Build a list of all LaneLines belonging to either section which are within this bounding box.
	vector<LaneSingleLine> candidateLines;
	for (size_t id=0; id<2; id++) {
		Section* currSect = id==0?currSectPair.first:currSectPair.second;
		if (currSect) {
			for (map<int, vector<Lane*> >::iterator laneIt=currSect->laneLinesAtNode.begin(); laneIt!=currSect->laneLinesAtNode.end(); laneIt++) {
				if (LanesWithinBounds(laneIt->second, startEndEdges)) {
					candidateLines.push_back(LaneSingleLine(laneIt->second));
				}
			}
		}
	}

	//Prune this list until all angles are within a certain threshold of each other
	TrimCandidateList(candidateLines, 0, threshhold);
	return candidateLines;
}



void CalculateSectionLanes(pair<Section*, Section*> currSectPair, const pair<Lane, Lane>& medianEndpoints, int singleLaneWidth)
{
	//First, we need a general idea of the angles in this Section.
	vector<LaneSingleLine> candidateLines = CalculateSectionGeneralAngleCandidateList(currSectPair, singleLaneWidth, 0.034906585); //Within about 2 degrees
	if (candidateLines.empty()) {
		return;
	}

	//Average over all angles
	double theta = 0.0;
	for (vector<LaneSingleLine>::iterator it=candidateLines.begin(); it!=candidateLines.end(); it++) {
		it->computeAndSaveAngle();
		theta += it->angle/candidateLines.size();
	}

	//Get the distance between these two nodes.
	double sectDist = dist(currSectPair.first->fromNode->xPos, currSectPair.first->fromNode->yPos, currSectPair.first->toNode->xPos, currSectPair.first->toNode->yPos);

	//Next, we simply draw lines from the previous node's lanes through this node's lanes.
	// All lines stop when they cross the line normal to this Section's angle (which is slightly
	// inaccurate if the lane rounds a corner, but we use different functionality to import accurate Lanes.)
	//TODO: For the start/end nodes, we should use the medianEndpoints provided, since these lanes won't end on the node exactly.
	//Note that adding/removing lanes complicates our algorithm slightly.
	//NOTE: This function is a bit coarse, since we're only hoping to rely on it for initial data.
	for (size_t i=0; i<2; i++) {
		//Create a vector going "left" from lane zero. We will use this to build new starting points.
		Section* currSect = i==0 ? currSectPair.first : currSectPair.second;
		if (!currSect) {
			continue;
		}
		double magX = cos(theta);
		double magY = sin(theta);
		if (i==1) {
			magX = -magX;
			magY = -magY;
		}
		DynamicVector originPt(currSect->fromNode->xPos, currSect->fromNode->yPos, currSect->fromNode->xPos+magX, currSect->fromNode->yPos+magY);
		originPt.flipNormal(false);

		//For each laneID, scale the originPt and go from there
		for (size_t laneID=0; laneID<=(size_t)currSect->numLanes; laneID++) {
			//Scale our vector
			originPt.makeUnit().scaleVect(singleLaneWidth);

			if (currSect) {
				//Ensure our vector is sized properly
				while (currSect->lanePolylinesForGenNode.size()<=laneID) {
					currSect->lanePolylinesForGenNode.push_back(std::vector<sim_mob::Point2D>());
				}

				//Create a vector to the ending point
				DynamicVector laneVect(originPt.getX(), originPt.getY(), originPt.getX()+magX, originPt.getY()+magY);
				laneVect.makeUnit().scaleVect(sectDist);

				//Add the starting point, ending point
				sim_mob::Point2D startPt((int)laneVect.getX(), (int)laneVect.getY());
				sim_mob::Point2D endPt((int)laneVect.getEndX(), (int)laneVect.getEndY());
				currSect->lanePolylinesForGenNode[laneID].push_back(startPt);
				currSect->lanePolylinesForGenNode[laneID].push_back(endPt);
			}

			//Scale the starting vector
			originPt.translateVect();
		}
	}
}





} //End anon namespace



//Somewhat complex algorithm for filtering our swirling vortex of Lane data down into a single
//  polyline for each Segment representing the median.
void sim_mob::aimsun::Loader::GenerateLinkLaneZero(const sim_mob::RoadNetwork& rn, Node* start, Node* end, set<Section*> linkSections)
{
	//Step 1: Retrieve candidate endpoints. For each Lane_Id in all Segments within this Link,
	//        get the point closest to the segment's start or end node. If this point is within X
	//        cm of the start/end, it becomes a candidate point.
	const double minCM = (75 * 100)/2; //75 meter diameter
	pair< vector<LaneSingleLine>, vector<LaneSingleLine> > candidates; //Start, End
	for (set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
		for (map<int, vector<Lane*> >::iterator laneIt=(*it)->laneLinesAtNode.begin(); laneIt!=(*it)->laneLinesAtNode.end(); laneIt++) {
			//We need at least one candidate
			if (laneIt->second.empty()) {
				continue;
			}

			double ptStart = getClosestPoint(laneIt->second, start->xPos, start->yPos);
			double ptEnd = getClosestPoint(laneIt->second, end->xPos, end->yPos);
			double minPt = ptStart<ptEnd ? ptStart : ptEnd;
			vector<LaneSingleLine>& minVect = ptStart<ptEnd ? candidates.first : candidates.second;
			if (minPt <= minCM) {
				minVect.push_back(LaneSingleLine(laneIt->second));
			}
		}
	}


	//Step 2: We now have to narrow these points down to NumLanes + 1 + 1 total points.
	//        NumLanes is calculated based on the number of lanes in the incoming and outgoing
	//        Section, +1 since each lane shares 2 points. The additional +1 is for Links
	//        with a median. Note that one-way Links only have NumLanes+1.
	//        Each Link may, of course, have less than the total number of points, which usually
	//        indicates missing data.
	pair< size_t, size_t > maxCandidates(0, 0); //start, end
	int extra1 = 1; //We will disable the +2 by default
	int extra2 = 1; //We will disable the +2 by default
	for (set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
		//"from" or "to" the start?
		if ((*it)->fromNode==start) {
			maxCandidates.first += (*it)->numLanes + extra1;
			extra1 = 0;
		} else if ((*it)->toNode==start) {
			maxCandidates.first += (*it)->numLanes + extra1;
			extra1 = 0;
		}

		//"from" or "to" the end?
		if ((*it)->fromNode==end) {
			maxCandidates.second += (*it)->numLanes + extra2;
			extra2 = 0;
		} else if ((*it)->toNode==end) {
			maxCandidates.second += (*it)->numLanes + extra2;
			extra2 = 0;
		}
	}

	//Perform the trimming
	TrimCandidateList(candidates.first, maxCandidates.first, 0.0);
	TrimCandidateList(candidates.second, maxCandidates.second, 0.0);


	//Step 2.5: If we don't have any candidates to work with, just use the RoadSegment polyline to generate the Lane geometry
	if (candidates.first.empty() || candidates.second.empty()) {
		return;
	}


	//TEMP: Lane width
	//TODO: Calculate dynamically, or pull from the database
	int singleLaneWidth = 300; //3m


	//Step 3: Take the first point on each of the "start" candidates, and the last point on each
	//        of the "end" candidates. These are the major points. If this number is equal to
	//        the maximum number of lines, then we take the center line as the median. Otherwise
	//        we take the average distance between the nearest and the farthest line. Of course,
	//        if there is only one segment outgoing/incoming, then we take the farthest line(s)
	//        since the median is not shared.
	// NOTE:  Currently, actually specifying a median with 2 lines is disabled, since too many lines
	//        has extra segments which would have registered as double-line medians.
	// NOTE:  The algorithm described above has to be performed for each Section, and then saved in the
	//        generated RoadSegment.
	// NOTE:  We also update the segment width.
	pair<Lane, Lane> medianEndpoints = ComputeMedianEndpoints(rn.drivingSide==DRIVES_ON_LEFT, start, end, candidates, maxCandidates, linkSections); //Start, end


	//Step 4: Now that we have the median endpoints, travel to each Segment Node and update this median information.
	//        This is made mildly confusing by the fact that each SegmentNode may represent a one-way or bi-directional street.
	pair<Section*, Section*> currSectPair(nullptr, nullptr); //Fwd/Reverse
	for (std::set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
		if ((*it)->fromNode->id==start->id) {
			currSectPair.first = *it;
		}
		if ((*it)->toNode->id==start->id) {
			currSectPair.second = *it;
		}
	}

	size_t maxLoops = linkSections.size() + 1;
	for (; currSectPair.first || currSectPair.second ;) { //Loop as long as we have data to operate on.
		//Compute and save lanes for this Section and its reverse
		CalculateSectionLanes(currSectPair, medianEndpoints, singleLaneWidth);
		if (currSectPair.first) {
			currSectPair.first->generatedSegment->lanePolylines_cached = currSectPair.first->lanePolylinesForGenNode;
		}
		if (currSectPair.second) {
			currSectPair.second->generatedSegment->lanePolylines_cached = currSectPair.second->lanePolylinesForGenNode;
		}

		//Get the next Section
		Section* prevFwd = currSectPair.first;
		currSectPair.first = nullptr;
		currSectPair.second = nullptr;
		for (std::set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
			//"Fwd" section is predefined.
			if ((*it)->fromNode->id==prevFwd->toNode->id && (*it)->toNode->id!=prevFwd->fromNode->id) {
				currSectPair.first = *it;
			}
		}
		if (currSectPair.first) {
			for (std::set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
				//"Rev" section is just fwd in reverse. We'll have to tidy this up later for roads that diverge.
				if ((*it)->toNode->id==currSectPair.first->fromNode->id && (*it)->fromNode->id==currSectPair.first->toNode->id) {
					currSectPair.second = *it;
				}
			}
		}

		//Avoid looping forever
		if (maxLoops-- == 0) {
			throw std::runtime_error("Error: Network contains RoadSegment loop.");
		}
	}
}




void sim_mob::aimsun::Loader::ProcessGeneralNode(sim_mob::RoadNetwork& res, Node& src)
{
	src.hasBeenSaved = true;

	sim_mob::Node* newNode = nullptr;
	if (!src.candidateForSegmentNode) {
		//This is an Intersection
		newNode = new sim_mob::Intersection();

		//Store it in the global nodes array
		res.nodes.push_back(dynamic_cast<MultiNode*>(newNode));
	} else {
		//Just save for later so the pointer isn't invalid
		newNode = new UniNode();
		res.segmentnodes.insert(dynamic_cast<UniNode*>(newNode));
	}

	//Always save the location
	newNode->location = new Point2D(src.getXPosAsInt(), src.getYPosAsInt());

	//For future reference
	src.generatedNode = newNode;
}


void sim_mob::aimsun::Loader::ProcessUniNode(sim_mob::RoadNetwork& res, Node& src)
{
	//Find 2 sections "from" and 2 sections "to".
	//(Bi-directional segments will complicate this eventually)
	//Most of the checks done here are already done earlier in the Loading process, but it doesn't hurt to check again.
	pair<Section*, Section*> fromSecs(nullptr, nullptr);
	pair<Section*, Section*> toSecs(nullptr, nullptr);
	for (vector<Section*>::iterator it=src.sectionsAtNode.begin(); it!=src.sectionsAtNode.end(); it++) {
		if ((*it)->TMP_ToNodeID==src.id) {
			if (!fromSecs.first) {
				fromSecs.first = *it;
			} else if (!fromSecs.second) {
				fromSecs.second = *it;
			} else {
				throw std::runtime_error("UniNode contains unexpected additional Sections leading TO.");
			}
		} else if ((*it)->TMP_FromNodeID==src.id) {
			if (!toSecs.first) {
				toSecs.first = *it;
			} else if (!toSecs.second) {
				toSecs.second = *it;
			} else {
				throw std::runtime_error("UniNode contains unexpected additional Sections leading FROM.");
			}
		} else {
			throw std::runtime_error("UniNode contains a Section which actually does not lead to/from that Node.");
		}
	}

	//Ensure at least one path was found, and a non-partial second path.
	if (!(fromSecs.first && toSecs.first)) {
		throw std::runtime_error("UniNode contains no primary path.");
	}
	if ((fromSecs.second && !toSecs.second) || (!fromSecs.second && toSecs.second)) {
		throw std::runtime_error("UniNode contains partial secondary path.");
	}

	//This is a simple Road Segment joint
	UniNode* newNode = dynamic_cast<UniNode*>(src.generatedNode);
	//newNode->location = new Point2D(src.getXPosAsInt(), src.getYPosAsInt());

	//Set locations (ensure unset locations are null)
	//Also ensure that we don't point backwards from the same segment.
	bool parallel = fromSecs.first->fromNode->id == toSecs.first->toNode->id;
	newNode->firstPair.first = fromSecs.first->generatedSegment;
	newNode->firstPair.second = parallel ? toSecs.second->generatedSegment : toSecs.first->generatedSegment;
	if (fromSecs.second && toSecs.second) {
		newNode->secondPair.first = fromSecs.second->generatedSegment;
		newNode->secondPair.second = parallel ? toSecs.first->generatedSegment : toSecs.second->generatedSegment;
	} else {
		newNode->secondPair = pair<RoadSegment*, RoadSegment*>(nullptr, nullptr);
	}

	//Save it for later reference
	//res.segmentnodes.insert(newNode);

	//TODO: Actual connector alignment (requires map checking)
	sim_mob::UniNode::buildConnectorsFromAlignedLanes(newNode, std::make_pair(0, 0), std::make_pair(0, 0));

	//This UniNode can later be accessed by the RoadSegment itself.
}


void sim_mob::aimsun::Loader::ProcessSection(sim_mob::RoadNetwork& res, Section& src)
{
	//Skip Sections which start at a non-intersection. These will be filled in later.
	if (src.fromNode->candidateForSegmentNode) {
		return;
	}

	//Process this section, and continue processing Sections along the direction of
	// travel until one of these ends on an intersection.
	//NOTE: This approach is far from foolproof; for example, if a Link contains single-directional
	//      Road segments that fail to match at every UniNode. Need to find a better way to
	//      group RoadSegments into Links, but at least this works for our test network.
	Section* currSect = &src;
	sim_mob::Link* ln = new sim_mob::Link();
	src.generatedSegment = new sim_mob::RoadSegment(ln);
	ln->roadName = currSect->roadName;
	ln->start = currSect->fromNode->generatedNode;
	set<RoadSegment*> linkSegments;

	//Make sure the link's start node is represented at the Node level.
	//TODO: Try to avoid dynamic casting if possible.
	dynamic_cast<MultiNode*>(src.fromNode->generatedNode)->roadSegmentsAt.insert(src.generatedSegment);

	for (;;) {
		//Update
		ln->end = currSect->toNode->generatedNode;

		//Now, check for segments going both forwards and backwards. Add both.
		for (size_t i=0; i<2; i++) {
			//Phase 1 = find a reverse segment
			Section* found = nullptr;
			if (i==0) {
				found = currSect;
			} else {
				for (vector<Section*>::iterator iSec=currSect->toNode->sectionsAtNode.begin(); iSec!=currSect->toNode->sectionsAtNode.end(); iSec++) {
					Section* newSec = *iSec;
					if (newSec->fromNode==currSect->toNode && newSec->toNode==currSect->fromNode) {
						found = newSec;
						break;
					}
				}
			}

			//Check: No reverse segment
			if (!found) {
				break;
			}

			//Check: not processing an existing segment
			if (found->hasBeenSaved) {
				throw std::runtime_error("Section processed twice.");
			}

			//Mark saved
			found->hasBeenSaved = true;

			//Check name
			if (ln->roadName != found->roadName) {
				throw std::runtime_error("Road names don't match up on RoadSegments in the same Link.");
			}

			//Prepare a new segment IF required, and save it for later reference (or load from past ref.)
			if (!found->generatedSegment) {
				found->generatedSegment = new sim_mob::RoadSegment(ln);
			}

			//Retrieve the generated segment
			sim_mob::RoadSegment* rs = found->generatedSegment;

			//Start/end need to be added properly
			rs->start = found->fromNode->generatedNode;
			rs->end = found->toNode->generatedNode;

			//Process
			rs->maxSpeed = found->speed;
			rs->length = found->length;
			for (int laneID=0; laneID<found->numLanes; laneID++) {
				rs->lanes.push_back(new sim_mob::Lane(rs, laneID));
			}
			rs->width = 0;

			//TODO: How do we determine if lanesLeftOfDivider should be 0 or lanes.size()
			//      In other words, how do we apply driving direction?
			//NOTE: This can be done easily later from the Link's point-of-view.
			rs->lanesLeftOfDivider = 0;
			linkSegments.insert(rs);
		}

		//Break?
		if (!currSect->toNode->candidateForSegmentNode) {
			//Make sure the link's end node is represented at the Node level.
			//TODO: Try to avoid dynamic casting if possible.
			dynamic_cast<MultiNode*>(currSect->toNode->generatedNode)->roadSegmentsAt.insert(currSect->generatedSegment);

			//Save it.
			ln->initializeLinkSegments(linkSegments);
			break;
		}


		//Increment.
		Section* nextSection = nullptr;
		for (vector<Section*>::iterator it2=currSect->toNode->sectionsAtNode.begin(); it2!=currSect->toNode->sectionsAtNode.end(); it2++) {
			//Our eariler check guarantees that there will be only ONE node which leads "from" the given segment "to" a node which is not the
			//  same node.
			if ((*it2)->fromNode==currSect->toNode && (*it2)->toNode!=currSect->fromNode) {
				if (nextSection) {
					throw std::runtime_error("UniNode has competing outgoing Sections.");
				}
				nextSection = *it2;
			}
		}
		if (!nextSection) {
			std::cout <<"PATH ERROR:\n";
			std::cout <<"  Starting at Node: " <<src.fromNode->id <<"\n";
			std::cout <<"  Currently at Node: " <<currSect->toNode->id <<"\n";
			throw std::runtime_error("No path reachable from RoadSegment.");
		}
		currSect = nextSection;
	}

	//Now add the link
	res.links.push_back(ln);

}




void sim_mob::aimsun::Loader::ProcessTurning(sim_mob::RoadNetwork& res, Turning& src)
{
	//Check
	if (src.fromSection->toNode->id != src.toSection->fromNode->id) {
		throw std::runtime_error("Turning doesn't match with Sections and Nodes.");
	}

	//Skip Turnings which meet at UniNodes; these will be handled elsewhere.
	sim_mob::Node* meetingNode = src.fromSection->toNode->generatedNode;
	if (dynamic_cast<UniNode*>(meetingNode)) {
		return;
	}

	//Essentially, just expand each turning into a set of LaneConnectors.
	//TODO: This becomes slightly more complex at RoadSegmentNodes, since these
	//      only feature one primary connector per Segment pair.
	for (int fromLaneID=src.fromLane.first; fromLaneID<=src.fromLane.second; fromLaneID++) {
		for (int toLaneID=src.toLane.first; toLaneID<=src.toLane.second; toLaneID++) {
			sim_mob::LaneConnector* lc = new sim_mob::LaneConnector();
			lc->laneFrom = src.fromSection->generatedSegment->lanes[fromLaneID];
			lc->laneTo = src.toSection->generatedSegment->lanes[toLaneID];

			//Expanded a bit...
			RoadSegment* key = lc->laneFrom->getRoadSegment();
			map<const RoadSegment*, set<LaneConnector*> >& connectors = dynamic_cast<MultiNode*>(src.fromSection->toNode->generatedNode)->connectors;
			connectors[key].insert(lc);
		}
	}

}



void sim_mob::aimsun::Loader::ProcessSectionPolylines(sim_mob::RoadNetwork& res, Section& src)
{
	//The start point is first
	// NOTE: We agreed earlier to include the start/end points; I think this was because it makes lane polylines consistent. ~Seth
	{
		sim_mob::Point2D pt(src.fromNode->generatedNode->location->getX(), src.fromNode->generatedNode->location->getY());
		src.generatedSegment->polyline.push_back(pt);
	}

	//Polyline points are sorted by their "distance from source" and then added.
	std::sort(src.polylineEntries.begin(), src.polylineEntries.end(), polyline_sorter);
	for (std::vector<Polyline*>::iterator it=src.polylineEntries.begin(); it!=src.polylineEntries.end(); it++) {
		//TODO: This might not trace the median, and the start/end points are definitely not included.
		sim_mob::Point2D pt((*it)->xPos, (*it)->yPos);
		src.generatedSegment->polyline.push_back(pt);
	}

	//The ending point is last
	sim_mob::Point2D pt(src.toNode->generatedNode->location->getX(), src.toNode->generatedNode->location->getY());
	src.generatedSegment->polyline.push_back(pt);
}




string sim_mob::aimsun::Loader::LoadNetwork(const string& connectionStr, map<string, string>& storedProcs, sim_mob::RoadNetwork& rn)
{
	try {
		//Temporary AIMSUN data structures
		map<int, Node> nodes;
		map<int, Section> sections;
		vector<Crossing> crossings;
		vector<Lane> lanes;
		map<int, Turning> turnings;
		multimap<int, Polyline> polylines;

		//Step One: Load
		LoadBasicAimsunObjects(connectionStr, storedProcs, nodes, sections, crossings, lanes, turnings, polylines);

		//Step Two: Translate
		DecorateAndTranslateObjects(nodes, sections, crossings, lanes, turnings, polylines);

		//Step Three: Save
		SaveSimMobilityNetwork(rn, nodes, sections, turnings, polylines);



	} catch (std::exception& ex) {
		return string(ex.what());
	}

	std::cout <<"AIMSUN Network successfully imported.\n";
	return "";
}

