/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HMLuaModel.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 10, 2013, 2:39 PM
 */

#include "HM_LuaModel.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"


using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace luabridge;
using std::vector;

HM_LuaModel::HM_LuaModel() : lua::LuaModel() {
}

HM_LuaModel::HM_LuaModel(const HM_LuaModel& orig) : lua::LuaModel(orig) {
}

HM_LuaModel::~HM_LuaModel() {
}

void HM_LuaModel::mapClasses() {
    getGlobalNamespace(state.get())
            .beginClass <ExpectationEntry> ("ExpectationEntry")
            .addConstructor <void (*) (void) > ()
            .addData("price", &ExpectationEntry::price)
            .addData("expectation", &ExpectationEntry::expectation)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <Unit> ("Unit")
            .addProperty("id", &Unit::GetId)
            .addProperty("buildingId", &Unit::GetBuildingId)
            .addProperty("typeId", &Unit::GetTypeId)
            .addProperty("postcodeId", &Unit::GetPostcodeId)
            .addProperty("floorArea", &Unit::GetFloorArea)
            .addProperty("storey", &Unit::GetStorey)
            .addProperty("rent", &Unit::GetRent)
            .addProperty("hedonicPrice", &Unit::GetHedonicPrice)
            .addProperty("askingPrice", &Unit::GetAskingPrice)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <Household> ("Household")
            .addProperty("id", &Household::getId)
            .addProperty("lifestyleId", &Household::getLifestyleId)
            .addProperty("unitId", &Household::getUnitId)
            .addProperty("ethnicityId", &Household::getEthnicityId)
            .addProperty("vehicleCategoryId", &Household::getVehicleCategoryId)
            .addProperty("size", &Household::getSize)
            .addProperty("children", &Household::getChildren)
            .addProperty("income", &Household::getIncome)
            .addProperty("housingDuration", &Household::getHousingDuration)
            .addProperty("workers", &Household::getWorkers)
            .addProperty("ageOfHead", &Household::getAgeOfHead)
            .endClass();
}

void HM_LuaModel::calulateUnitExpectations(const Unit& unit, int timeOnMarket,
        vector<ExpectationEntry>& outValues) const{
    LuaRef funcRef = getGlobal(state.get(), "calulateUnitExpectations");
    LuaRef retVal = funcRef(unit, timeOnMarket);
    if (retVal.isTable()) {
        for (int i = 1; i <= retVal.length(); i++) {
            ExpectationEntry entry;
            entry.price = retVal[i].cast<ExpectationEntry>().price;
            entry.expectation = retVal[i].cast<ExpectationEntry>().expectation;
            outValues.push_back(entry);
        }
    }
}

double HM_LuaModel::calculateHedonicPrice(const Unit& unit) const{
    LuaRef funcRef = getGlobal(state.get(), "calculateHedonicPrice");
    LuaRef retVal = funcRef(unit);
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}

double HM_LuaModel::calculateSurplus(const Unit& unit, int unitBids) const{
    LuaRef funcRef = getGlobal(state.get(), "calculateSurplus");
    LuaRef retVal = funcRef(unit, unitBids);
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}

double HM_LuaModel::calulateWP(const Household& hh, const Unit& unit) const{
    LuaRef funcRef = getGlobal(state.get(), "calculateWP");
    LuaRef retVal = funcRef(hh, unit);
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}