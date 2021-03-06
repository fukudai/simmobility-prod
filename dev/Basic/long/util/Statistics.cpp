//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Statistics.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on June 14, 2013, 11:15 AM
 */
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include "Statistics.hpp"
#include "logging/Log.hpp"
#include "Common.hpp"

using std::endl;
using std::map;
using std::pair;
using std::string;
using namespace sim_mob;
using namespace sim_mob::long_term;

typedef boost::unordered_map<int, long> StatsMap;
typedef pair<int, long> StatsMapEntry;

StatsMap statistics;
boost::shared_mutex statisticsMutex;

inline string toString(Statistics::StatsParameter param) {
    switch (param) {
        case Statistics::N_BIDS: return "Total number of bids";
        case Statistics::N_ACCEPTED_BIDS: return "Total number of accepted bids";
        case Statistics::N_BID_RESPONSES: return "Total number of bid responses";
        case Statistics::N_WAITING_TO_MOVE: return "Total number of waiting to move";
        default: return "";
    }
}

void Statistics::increment(Statistics::StatsParameter param) {
    increment(param, 1);
}

void Statistics::decrement(Statistics::StatsParameter param) {
    decrement(param, 1);
}

void Statistics::decrement(Statistics::StatsParameter param, long value) {
    increment(param, value * (-1));
}

void Statistics::increment(Statistics::StatsParameter param, long value) {
    boost::upgrade_lock<boost::shared_mutex> up_lock(statisticsMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    StatsMap::iterator mapItr = statistics.find(param);
    if (mapItr != statistics.end()) {
        (mapItr->second) += value;
    } else {
        statistics.insert(StatsMapEntry(param, value));
    }
}

void Statistics::reset(StatsParameter param)
{
    boost::upgrade_lock<boost::shared_mutex> up_lock(statisticsMutex);
        boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
        StatsMap::iterator mapItr = statistics.find(param);
        if (mapItr != statistics.end()) {
            (mapItr->second) = 0;
        } else {
            statistics.insert(StatsMapEntry(param, 0));
        }
}

void Statistics::print() {
    boost::upgrade_lock<boost::shared_mutex> up_lock(statisticsMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    for (StatsMap::iterator itr = statistics.begin(); itr != statistics.end(); itr++) {
        string paramName = toString((Statistics::StatsParameter)(itr->first));
        long value = itr->second;
        PrintOut(paramName << ": " << value << endl);
    }
}

long Statistics::getValue(Statistics::StatsParameter param) {
    boost::upgrade_lock<boost::shared_mutex> up_lock(statisticsMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    long value = 0;
    for (StatsMap::iterator itr = statistics.begin(); itr != statistics.end(); itr++) {
        if(itr->first == param)
        {
            return itr->second;
        }
    }

    return 0;
}
