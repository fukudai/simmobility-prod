/*
 * IncidentResponse.cpp
 *
 *  Created on: Oct 2, 2013
 *      Author: zhang
 */

#include "IncidentResponse.hpp"
#include <cmath>

namespace sim_mob {

IncidentResponse::IncidentResponse() : currentPlan(INCIDENT_CLEARANCE), startFrameTick(0), curFrameTick(0), speedLimit(0) {
	// TODO Auto-generated constructor stub

}

IncidentResponse::~IncidentResponse() {
	// TODO Auto-generated destructor stub
}

bool IncidentResponse::insertIncident(const Incident* inc){
	if( currentIncidents.count(inc->incidentId) > 0 ) {
		return false;
	}

	currentIncidents[inc->incidentId] = inc;
	if( speedLimit<=0 ){
		speedLimit = inc->speedlimit;
	}
	else{
		speedLimit = std::min(speedLimit, inc->speedlimit);
	}
	return true;
}

bool IncidentResponse::removeIncident(const Incident* inc){

	for(std::map<unsigned int, const Incident*>::iterator incIt = currentIncidents.begin(); incIt != currentIncidents.end(); incIt++) {
		if( (*incIt).first == inc->incidentId ){
			currentIncidents.erase(inc->incidentId);
			return true;
		}
	}

	return false;
}

void IncidentResponse::makeResponsePlan(timeslice* now, const RoadSegment* currentRoad){

	if(currentIncidents.size() > 0 ){
		currentPlan = INCIDENT_SLOWDOWN_OR_STOP;
	}
	else {
		currentPlan = INCIDENT_CLEARANCE;
	}

}

void IncidentResponse::resetStatus(){
	currentPlan = INCIDENT_CLEARANCE;
	nextLaneIndex = -1;
	speedLimit = -1;
	//startFrameTick = 0;
	//curFrameTick = 0;
}


} /* namespace sim_mob */
