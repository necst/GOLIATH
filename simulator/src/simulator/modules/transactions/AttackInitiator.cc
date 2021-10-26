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

#include "AttackInitiator.h"

Define_Module(framework::AttackInitiator);

namespace framework {
void AttackInitiator::initialize() {
    ValidTransactionGenerator::initialize();

    manager = dynamic_cast<DoubleTypeTraCIManagerLaunchd*>(getModuleByPath("<root>.manager"));
    ASSERT(manager != nullptr);
    addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    ASSERT(addressTable != nullptr);

    startAt = par("startAt");
    stopAt = par("stopAt");
    ASSERT(stopAt >= startAt);
    ASSERT(startAt >= simTime());
    attackMsg = new cMessage("startAttack");
    count = 0;
    attempts = 0;
    scheduleAt(startAt, attackMsg);
}

void AttackInitiator::finish() {
    ASSERT(simTime() >= startAt);
    cancelAndDelete(attackMsg);
    recordScalar("attack initiation", count, "trans");
    recordScalar("attack initiation attempts", attempts);
    recordScalar("okk with time", simTime() >= startAt);
}

void AttackInitiator::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage() && msg == attackMsg) {
        ++attempts;
        NeighbourDiscovery* beacon = new NeighbourDiscovery();
        beacon->setSender(blockchain->getAddress());
        beacon->setLength(sizeof(LAddress::L3Type) * 2 + sizeof(Coord) + sizeof(simtime_t));
        beacon->setSenderPosition(positionTable->getPosition(blockchain->getAddress()));
        beacon->setSentAt(simTime());
        for(auto it: addressTable->getAllModules()) {
            if(blockchain->getAddress() != it.first
                    && !manager->isMainType(addressTable->getNameFor(it.first))
                    && it.second.second != nullptr) {
                // create transaction from this node to malicious group member
                std::shared_ptr<Transaction> t1 (
                    new Transaction(blockchain->getAddress(), it.first,
                            positionTable->getPosition(blockchain->getAddress()),
                            par("range").intValue(), simTime()));

                blockchain->handleTransaction(t1);
                // notify malicious group member to start the attack
                beacon->setReceiver(it.first);
                sendDirect(beacon->dup(), it.second.second, "dsrcIn");
                ++count;
            }
        }
        delete beacon;
        if(simTime() < stopAt)
            scheduleAt(simTime() + timer, attackMsg);
    } else
        ValidTransactionGenerator::handleMessage(msg);
}
}
