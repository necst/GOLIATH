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

#include "DeterministicHarvester.h"

Define_Module(framework::DeterministicHarvester);

namespace framework {
void DeterministicHarvester::initialize()
{
    blocktime = SimTime(par("blocktime").intValue(), SimTimeUnit::SIMTIME_S);
    harvestSignal = registerSignal("harvestSignal");
    timer = new cMessage();
    scheduleAt(simTime() + blocktime, timer);
}

void DeterministicHarvester::finish() {
    cancelAndDelete(timer);
}

void DeterministicHarvester::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage() && msg == timer) {
        auto blockchain =
                dynamic_cast<Blockchain*>(getParentModule()->getSubmodule("blockchain"));
        //reschedule timer
        scheduleAt(simTime() + blocktime, msg);
        emit(harvestSignal, canHarvestBlock(blockchain->getMostRecentView()));
    } else
        delete msg;
}

bool DeterministicHarvester::canHarvestBlock(const NetworkView& view) const {
    auto blockchain = dynamic_cast<Blockchain*>(getParentModule()->getSubmodule("blockchain"));
    auto address = blockchain->getAddress();

    simtime_t bound = SimTime(par("blocktime").intValue(), SimTimeUnit::SIMTIME_S);
    // scale bound to keep track of previous attempts
    bound *= (view.getAttempt() + 1);
    simtime_t timeSinceLastHarvest = simTime() - blockchain->getNthMostRecentBlock(0)->getCreationTime();

    if(timeSinceLastHarvest >= bound)
        return view.isHarvester(address);
    return false;
}

void DeterministicHarvester::reset() {
    Enter_Method_Silent();
    cancelEvent(timer);
    scheduleAt(simTime() + blocktime, timer);

}

}
