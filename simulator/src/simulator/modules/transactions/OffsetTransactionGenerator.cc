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

#include "OffsetTransactionGenerator.h"
#include "veins/base/utils/FindModule.h"

Define_Module(framework::OffsetTransactionGenerator);

namespace framework {

void OffsetTransactionGenerator::initialize() {
    MaliciousTransactionGenerator::initialize();
    maxOffset = par("maxOffset");
    minOffset = par("minOffset");
    changeOffset = par("changeOffset");
    ASSERT(maxOffset >= minOffset);
    setRandomOffset(minOffset, maxOffset);
    answerBack = true;
}

void OffsetTransactionGenerator::setRandomOffset(int minOffset, int maxOffset) {
    auto direction = getRNG(0)->doubleRand() * 2 * M_PI;
    auto module = minOffset + getRNG(0)->doubleRandIncl1() * (maxOffset - minOffset);
    Coord offset(module, 0);
    this->offset = offset.rotatedYaw(direction);
}

std::vector<std::shared_ptr<const Transaction>> OffsetTransactionGenerator::makeTransactions() {
    // nothing to do
    return {};
}

Coord OffsetTransactionGenerator::getPosition() {
    return positionTable->getPosition(blockchain->getAddress()) + offset;
}

void OffsetTransactionGenerator::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        NeighbourDiscovery* beacon = new NeighbourDiscovery();
        beacon->setSender(blockchain->getAddress());
        beacon->setReceiver(LAddress::L3BROADCAST());
        beacon->setLength(sizeof(LAddress::L3Type) * 2 + sizeof(Coord) + sizeof(simtime_t));
        // get a new offset if set so
        if(changeOffset)
            setRandomOffset(minOffset, maxOffset);

        beacon->setSenderPosition(positionTable->getPosition(blockchain->getAddress()));
        beacon->setSentAt(simTime());

        auto app = FindModule<>::findHost(this);
        app = app->getSubmodule("app")->getSubmodule("application");

        auto application = dynamic_cast<Application*>(app);
        application->sendWaveMessage(beacon);

        scheduleAt(simTime() + timer, msg);
    } else
        delete msg;
}

void OffsetTransactionGenerator::handleReply(NeighbourReply* msg) {
    std::shared_ptr<Transaction> t1 (
        new Transaction(blockchain->getAddress(), msg->getSender(),
                positionTable->getPosition(blockchain->getAddress()) + offset,
                range, simTime()));

    blockchain->handleTransaction(t1);
    delete msg;
}

}
