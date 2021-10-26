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

#ifndef __SIMULATOR_FIXEDREWARD_H_
#define __SIMULATOR_FIXEDREWARD_H_

#include <omnetpp.h>

#include "simulator/blockchain/RewardSystem.h"

using namespace omnetpp;

namespace framework {
class FixedReward : public cSimpleModule, public RewardSystem
{
protected:
    int reward;
    int blockReward;
    int penalty;
    int blockPenalty;
    int maxReward;
    int startingReputation;
    int rsuStartingReputation;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
public:
    virtual int getRewardFor(Blockchain* blockchain,
            const veins::LAddress::L3Type& id) override;
    virtual int getPenaltyFor(Blockchain* blockchain,
            const veins::LAddress::L3Type& id) override;
    virtual int getHarvesterRewardFor(Blockchain* blockchain,
            const veins::LAddress::L3Type& id) override;
    virtual int getHarvesterPenaltyFor(Blockchain* blockchain,
            const veins::LAddress::L3Type& id) override;
    virtual int getStartingReputation() override;
    virtual int getRSUStartingReputation() override;
    virtual int getMaxReward() override;
};
}
#endif
