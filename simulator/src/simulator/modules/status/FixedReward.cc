//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "FixedReward.h"

Define_Module(framework::FixedReward);

namespace framework {
void FixedReward::initialize()
{
    reward = par("reward");
    ASSERT(reward >= 0);
    blockReward = par("blockReward");
    ASSERT(blockReward >= 0);
    penalty = par("penalty");
    ASSERT(penalty >= 0);
    blockPenalty = par("blockPenalty");
    ASSERT(blockPenalty >= 0);
    startingReputation = par("startingReward");
    ASSERT(startingReputation >= 0);
    rsuStartingReputation = par("rsuStartingReward");
    ASSERT(rsuStartingReputation >= 0);
    maxReward = par("maxReward");
    ASSERT(maxReward >= 0);
}

void FixedReward::handleMessage(cMessage *msg)
{
    // nothing to do
}


int FixedReward::getRewardFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return reward;
}

int FixedReward::getPenaltyFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return penalty;
}

int FixedReward::getHarvesterRewardFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return blockReward;
}

int FixedReward::getHarvesterPenaltyFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return blockPenalty;
}

int FixedReward::getStartingReputation() {
    return startingReputation;
}

int FixedReward::getRSUStartingReputation() {
    return rsuStartingReputation;
}

int FixedReward::getMaxReward() {
    return maxReward;
}

}
