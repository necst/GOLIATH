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

#include "ValidTransactionGenerator.h"

#include "simulator/blockchain/Blockchain.h"
#include "simulator/application/Application.h"
#include "simulator/networking/PositionTable.h"
#include "simulator/messages/NeighbourDiscovery_m.h"
#include "simulator/messages/NeighbourReply_m.h"

#include "veins/base/utils/FindModule.h"

Define_Module(framework::ValidTransactionGenerator);

namespace framework {
void ValidTransactionGenerator::initialize()
{
    timer = SimTime(par("timer").doubleValue(), SimTimeUnit::SIMTIME_S);
    blockchain = dynamic_cast<Blockchain*>(getParentModule()->getSubmodule("blockchain")->getSubmodule("blockchain"));
    ASSERT(blockchain != nullptr);
    positionTable = dynamic_cast<PositionTable*>(getModuleByPath("<root>.positionTable"));
    ASSERT(positionTable != nullptr);
    searchMsg = new cMessage();
    scheduleAt(simTime() + timer
            /*+ uniform(SimTime::ZERO, SimTime(100, SimTimeUnit::SIMTIME_MS))*/,
            searchMsg);
    /*syncMsg = new cMessage();
    scheduleAt(simTime() + 5 * timer, syncMsg);*/
}

void ValidTransactionGenerator::handleMessage(cMessage *msg)
{
    if(msg == searchMsg) {
        NeighbourDiscovery* beacon = new NeighbourDiscovery();
        beacon->setSender(blockchain->getAddress());
        beacon->setReceiver(LAddress::L3BROADCAST());
        beacon->setLength(sizeof(LAddress::L3Type) * 2 + sizeof(Coord) + sizeof(simtime_t));
        beacon->setSenderPosition(positionTable->getPosition(blockchain->getAddress()));
        beacon->setSentAt(simTime());

        auto app = FindModule<>::findHost(this);
        app = app->getSubmodule("app")->getSubmodule("application");

        auto application = dynamic_cast<Application*>(app);
        application->sendWaveMessage(beacon);

        scheduleAt(simTime() + timer, msg);
        return;

    /*} else if(msg == syncMsg) {
        syncronize();*/
    } else
        delete msg;
}

void ValidTransactionGenerator::handleRequest(NeighbourDiscovery* msg) {
    // send reply
    NeighbourReply* reply = new NeighbourReply();
    reply->setSender(blockchain->getAddress());
    reply->setReceiver(msg->getSender());
    reply->setLength(sizeof(LAddress::L3Type) * 2 + sizeof(Coord) + sizeof(simtime_t));
    reply->setTargetPosition(positionTable->getPosition(blockchain->getAddress()));
    reply->setSentAt(simTime());

    auto app = FindModule<>::findHost(this);
    app = app->getSubmodule("app")->getSubmodule("application");

    auto application = dynamic_cast<Application*>(app);
    application->sendWaveMessage(reply);

    delete msg;
}

void ValidTransactionGenerator::handleReply(NeighbourReply* msg) {
    /* generate transaction from this node to who has replied
     * obs: to receive a reply the other node must be in range anyway,
     * so the transaction is valid. Don't use message timestamp as
     * it can be unreliable
     */
    std::shared_ptr<Transaction> t1 (
        new Transaction(blockchain->getAddress(), msg->getSender(),
                positionTable->getPosition(blockchain->getAddress()),
                par("range").intValue(), simTime()));

    blockchain->handleTransaction(t1);
    //buffer.push_back(t1);
    delete msg;
}
/*
void ValidTransactionGenerator::synchronize() {
    std::vector<std::shared_ptr<const Transaction>> vec;
    std::move(buffer.begin(), buffer.end(), std::back_inserter(vec));
    buffer.clear();
    using trans_t = std::vector<std::shared_ptr<const Transaction>>::value_type;
    std::sort(vec.begin(), vec.end(), [] (trans_t t1, trans_t t2) {return *t2 < *t1;});

    blockchain->handleTransactions(vec);
}*/

}
