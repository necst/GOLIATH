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

#include "DynamicPenalty.h"

Define_Module(framework::DynamicPenalty);

namespace framework {
void DynamicPenalty::initialize()
{
    reward = par("reward");
    ASSERT(reward >= 0);
    blockReward = par("blockReward");
    ASSERT(blockReward >= 0);
    basePenalty = par("penalty");
    ASSERT(basePenalty >= 0);
    baseBlockPenalty = par("blockPenalty");
    ASSERT(baseBlockPenalty >= 0);
    scale = par("scaleFactor");
    ASSERT(scale >= 0);
    startingReputation = par("startingReward");
    ASSERT(startingReputation >= 0);
    rsuStartingReputation = par("rsuStartingReward");
    ASSERT(rsuStartingReputation >= 0);
    maxReward = par("maxReward");
    ASSERT(maxReward >= 0);
}

void DynamicPenalty::handleMessage(cMessage *msg)
{
    // nothing to do
}


int DynamicPenalty::getRewardFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return reward;
}

int DynamicPenalty::getPenaltyFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    // penalty = basePenalty * 2^(#negative variation in a row)
    auto block1 = blockchain->getNthMostRecentBlock(0);
    auto block2 = blockchain->getNthMostRecentBlock(1);
    int deltaReputation = 0;
    if(block1 && block2) {
        auto it1 = block1->getAllStatuses().find(id);
        auto it2 = block2->getAllStatuses().find(id);
        if(it1 != block1->getAllStatuses().end() && it2 != block2->getAllStatuses().end()) {
            deltaReputation = it1->second.reputation - it2->second.reputation;
        }
    }
    int penalty = basePenalty;
    if(deltaReputation < 0) {
        penalty *= deltaReputation / -basePenalty * scale;
    }
    return penalty;
}

int DynamicPenalty::getHarvesterRewardFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return blockReward;
}

int DynamicPenalty::getHarvesterPenaltyFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    // penalty = basePenalty * 2^(#negative variation in a row)
    auto block1 = blockchain->getNthMostRecentBlock(0);
    auto block2 = blockchain->getNthMostRecentBlock(1);
    int deltaReputation = 0;
    if(block1 && block2) {
        auto it1 = block1->getAllStatuses().find(id);
        auto it2 = block2->getAllStatuses().find(id);
        if(it1 != block1->getAllStatuses().end() && it2 != block2->getAllStatuses().end()) {
            deltaReputation = it1->second.reputation - it2->second.reputation;
        }
    }
    int penalty = baseBlockPenalty;
    if(deltaReputation < 0) {
        penalty *= deltaReputation / -baseBlockPenalty * scale;
    }
    return penalty;
}

int DynamicPenalty::getStartingReputation() {
    return startingReputation;
}

int DynamicPenalty::getRSUStartingReputation() {
    return rsuStartingReputation;
}

int DynamicPenalty::getMaxReward() {
    return maxReward;
}
}
