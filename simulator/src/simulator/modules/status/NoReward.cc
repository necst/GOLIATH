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

#include "NoReward.h"

Define_Module(framework::NoReward);

namespace framework {
void NoReward::initialize()
{
    // Do nothing
}

void NoReward::handleMessage(cMessage *msg)
{
    // Do nothing
}


int NoReward::getRewardFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return 0;
}

int NoReward::getPenaltyFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return 0;
}

int NoReward::getHarvesterRewardFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return 0;
}

int NoReward::getHarvesterPenaltyFor(Blockchain* blockchain, const veins::LAddress::L3Type& id) {
    return 0;
}

int NoReward::getStartingReputation() {
    return 1;
}

int NoReward::getRSUStartingReputation() {
    return 1;
}

int NoReward::getMaxReward() {
    return 1;
}

}
