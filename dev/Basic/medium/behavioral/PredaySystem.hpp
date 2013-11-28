//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
 * SystemOfModels.hpp
 *
 *  Created on: Nov 7, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>

#include "behavioral/lua/PredayLuaModel.hpp"
#include "params/PersonParams.hpp"
#include "PredayClasses.hpp"
#include "database/PopulationDao.hpp"
#include "database/dao/MongoDao.hpp"

namespace sim_mob {
namespace medium {

/**
 * Class for pre-day behavioral system of models.
 * Defines the sequence of models to be invoked and also handles dependencies between the models.
 * The models are specified by modelers in an external scripting language is invoked via this class.
 *
 * \note The only scripting language that is currently supported is Lua
 *
 * \author Harish Loganathan
 */
class PredaySystem {
public:
	PredaySystem(PersonParams& personParams);
	virtual ~PredaySystem();

	/**
	 * The sequence of models to be invoked for a person is coded in this function.
	 */
	void planDay();

private:
	/**
	 * Predicts the mode of travel for a tour.
	 * Executed for tours with usual location (usual work or education).
	 */
	void predictTourMode(Tour& tour);

	/**
	 * Predicts the mode and destination together for tours to unusual locations
	 */
	void predictTourModeDestination(Tour& tour);

	/**
	 * Predicts the time period that will be allotted for the primary activity of a tour.
	 */
	std::string& predictTourTimeOfDay(Tour& tour);

	/**
	 * Generates intermediate stops of types predicted by the day pattern model before and after the primary activity of a tour.
	 */
	void generateIntermediateStops(Tour& tour);

	/**
	 * Predicts the mode and destination together for stops.
	 */
	void predictStopModeDestination(Stop& stop);

	/**
	 * Predicts the arrival time for stops before the primary activity.
	 * Predicts the departure time for stops after the primary activity.
	 */
	void predictStopTimeOfDay(Stop& stop, bool isBeforePrimary);

	/**
	 * Calculates the arrival time for stops in the second half tour.
	 */
	void calculateArrivalTime(Stop& currentStop, Stop& prevStop);

	/**
	 * Calculates the departure time for stops in the first half tour.
	 */
	void calculateDepartureTime(Stop& currentStop, Stop& nextStop);

	/**
	 * Calculates the time to leave home for starting a tour.
	 */
	void calculateTourStartTime(Tour& tour);

	/**
	 * Calculates the time when the person reaches home at the end of the tour.
	 */
	void calculateTourEndTime(Tour& tour);

	/**
	 * constructs tour objects based on predicted number of tours. Puts the tour objects in tours deque.
	 */
	void constructTours(PredayLuaModel& predayLuaModel);

	/**
	 * The parameters for a person is obtained from the population and set in personParams.
	 */
    PersonParams personParams;

    /**
     * list of tours for this person
     */
    std::deque<Tour*> tours;

    /**
     * The predicted day pattern for the person indicating whether the person wants to make tours and stops of each type (Work, Education, Shopping, Others).
     */
    boost::unordered_map<std::string, bool> dayPattern;

    /**
     * The predicted number of tours for each type of tour - Work, Education, Shopping, Others.
     */
    boost::unordered_map<std::string, int> numTours;

    /**
     * Data access objects for mongo
     */
    boost::unordered_map<std::string, db::MongoDao*> mongoDao;

};

} // end namespace medium
} // end namespace sim_mob
