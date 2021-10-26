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

#include "TrafficJamGenerator.h"

Define_Module(framework::TrafficJamGenerator);

namespace framework {
void TrafficJamGenerator::initialize() {
    MaliciousTransactionGenerator::initialize();

    generateTransactions = false;
    position = Coord(0, 0);
    answerBack = false;
    count = 0;

    manager = dynamic_cast<DoubleTypeTraCIManagerLaunchd*>(getModuleByPath("<root>.manager"));
    ASSERT(manager != nullptr);
}
void TrafficJamGenerator::finish() {
    recordScalar("generated mal tr", count, "trans");
}

std::vector<std::shared_ptr<const Transaction>> TrafficJamGenerator::makeTransactions() {
    if(!generateTransactions)
        return {};

    std::vector<TransactionPtr> transactions;
    auto address = blockchain->getAddress();
    for(auto it: addressTable->getAllModules()) {
        if(address != it.first && !manager->isMainType(addressTable->getNameFor(it.first))) {
            transactions.emplace_back(new Transaction(
                    address,
                    it.first,
                    getPosition(),
                    range,
                    simTime()
            ));
            ++count;
        }
    }
    return transactions;
}

Coord TrafficJamGenerator::getPosition() {
    return position;
}

void TrafficJamGenerator::handleRequest(NeighbourDiscovery* msg) {
    EV << "starting attack" << std::endl;
    position = msg->getSenderPosition();
    double magnitude = range * getRNG(0)->doubleRand() / 2;
    Coord offset(magnitude, 0);
    double theta = getRNG(0)->doubleRand() * 2 * M_PI;
    position += offset.rotatedYaw(theta);
    generateTransactions = true;

    // create transaction from this node to attack initiator
    std::shared_ptr<Transaction> t1 (
        new Transaction(blockchain->getAddress(), msg->getSender(),
                getPosition(),
                par("range").intValue(), simTime()));
    blockchain->handleTransaction(t1);

    delete msg;
}

}
