/*
 * MaliciousTransactionGenerator.cc
 *
 *  Created on: Jul 28, 2020
 *      Author: dvdmff
 */

#include "MaliciousTransactionGenerator.h"

#include "simulator/application/Application.h"

#include "veins/base/utils/FindModule.h"

namespace framework {

void MaliciousTransactionGenerator::initialize() {
    timer = SimTime(par("timer").doubleValue(), SimTimeUnit::SIMTIME_S);
    blockchain = dynamic_cast<Blockchain*>(getModuleByPath("^.blockchain.blockchain"));
    ASSERT(blockchain != nullptr);
    positionTable = dynamic_cast<PositionTable*>(getModuleByPath("<root>.positionTable"));
    ASSERT(positionTable != nullptr);
    addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    ASSERT(addressTable != nullptr);
    world = dynamic_cast<BaseWorldUtility*>(getModuleByPath("<root>.world"));
    ASSERT(world != nullptr);
    answerBack= par("answerBack").boolValue();
    transactionsPerTurn = par("transactionsPerTurn");
    range = par("range");

    cMessage* msg = new cMessage();
    scheduleAt(simTime() + timer, msg);
}

void MaliciousTransactionGenerator::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        // Don't bother sending beacons, just generate malicious transaction periodically

        auto ts = makeTransactions();
        for(auto t: ts)
        //if((bool)t)
            blockchain->handleTransaction(t);
        scheduleAt(simTime() + timer, msg);
    } else
        delete msg;
}


void MaliciousTransactionGenerator::handleRequest(NeighbourDiscovery* msg) {
    if(answerBack) {
        // send reply with wrong position data
        NeighbourReply* reply = new NeighbourReply();
        reply->setSender(blockchain->getAddress());
        reply->setReceiver(msg->getSender());
        reply->setLength(sizeof(LAddress::L3Type) * 2 + sizeof(Coord) + sizeof(simtime_t));
        reply->setTargetPosition(getPosition());
        reply->setSentAt(simTime());

        auto app = FindModule<>::findHost(this);
        app = app->getSubmodule("app")->getSubmodule("application");

        auto application = dynamic_cast<Application*>(app);
        application->sendWaveMessage(reply);
    }
    delete msg;
}

void MaliciousTransactionGenerator::handleReply(NeighbourReply* msg) {
    // nothing to do, just delete message
    delete msg;
}


} /* namespace framework */
