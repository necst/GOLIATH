/*
 * RewardSystem.h
 *
 *  Created on: Jun 30, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_REWARDSYSTEM_H_
#define SIMULATOR_BLOCKCHAIN_REWARDSYSTEM_H_

#include "Blockchain.h"

namespace framework {

class RewardSystem {
public:
    virtual int getRewardFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) = 0;
    virtual int getPenaltyFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) = 0;
    virtual int getHarvesterRewardFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) = 0;
    virtual int getHarvesterPenaltyFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) = 0;
    virtual int getStartingReputation() = 0;
    virtual int getRSUStartingReputation() = 0;
    virtual int getMaxReward() = 0;
};

}


#endif /* SIMULATOR_BLOCKCHAIN_REWARDSYSTEM_H_ */
