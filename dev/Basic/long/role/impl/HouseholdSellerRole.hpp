/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdSellerRole.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 5:13 PM
 */
#pragma once

#include <boost/unordered_map.hpp>
#include "database/entity/Bid.hpp"
#include "role/LT_Role.hpp"
#include "database/entity/Household.hpp"
#include "core/HousingMarket.hpp"
#include "database/entity/housing-market/SellerParams.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseholdAgent;

        /**
         * Household Seller role.
         *
         * Seller will receive N bids each day and it will choose 
         * the maximum bid *of the time unit* (in this case is DAY) 
         * that satisfies the seller's asking price.
         */
        class HouseholdSellerRole : public LT_AgentRole<HouseholdAgent> {
        public:
            HouseholdSellerRole(HouseholdAgent* parent, Household* hh,
                    const SellerParams& params, HousingMarket* market);
            virtual ~HouseholdSellerRole();

            /**
             * Method that will update the seller on each tick.
             * @param currTime
             */
            virtual void Update(timeslice currTime);
        protected:

            /**
             * Inherited from LT_Role
             */
            virtual void HandleMessage(messaging::MessageReceiver::MessageType type,
                    messaging::MessageReceiver& sender, const messaging::Message& message);

        private:
            
            /**
             * Struct to store a expectation data.
             */
            typedef struct ExpectationEntry_ {
                double price;
                double expectation;
            } ExpectationEntry;
            
            /**
             * Decides over a given bid for a given unit.
             * @param bid given by the bidder.
             * @return true if accepts the bid or false otherwise.
             */
            bool Decide(const Bid& bid, const ExpectationEntry& entry);
            
            /**
             * Adjust the unit parameters for the next bids. 
             * @param unit
             */
            void AdjustUnitParams(Unit& unit);
            
            /**
             * Notify the bidders that have their bid were accepted.
             */
            void NotifyWinnerBidders();
            
            /**
             * Adjust parameters of all units that were not selled.
             */
            void AdjustNotSelledUnits();
            
            /**
             * Calculates the unit expectations to the maximum period of time 
             * that the seller is expecting to be in the market.
             * @param unit to cal
             */
            void CalculateUnitExpectations(const Unit& unit);
        
            /**
             * Gets current expectation entry for given unit.
             * @param unit to get the expectation.
             * @param outEntry (outParameter) to fill with the expectation. 
             *        If it not exists the values should be 0.
             * @return true if exists valid expectation, false otherwise.
             */
            bool GetCurrentExpectation(const Unit& unit, ExpectationEntry& outEntry);
        private:

            typedef std::vector<ExpectationEntry> ExpectationList;
            typedef std::pair<UnitId, ExpectationList> ExpectationMapEntry; 
            typedef boost::unordered_map<UnitId, ExpectationList> ExpectationMap; 
            typedef boost::unordered_map<UnitId, Bid> Bids;
            typedef std::pair<UnitId, Bid> BidEntry;
            
            friend class HouseholdAgent;
            HousingMarket* market;
            Household* hh;
            SellerParams params;
            timeslice currentTime;
            volatile bool hasUnitsToSale;
            //Current max bid information.
            Bids maxBidsOfDay;
            ExpectationMap unitExpectations;
        };
    }
}