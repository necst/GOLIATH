/*
 * MaliciousTransactionGenerator.h
 *
 *  Created on: Jul 28, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_MODULES_TRANSACTIONS_MALICIOUSTRANSACTIONGENERATOR_H_
#define SIMULATOR_MODULES_TRANSACTIONS_MALICIOUSTRANSACTIONGENERATOR_H_

#include <omnetpp.h>

#include "simulator/blockchain/TransactionGenerator.h"
#include "simulator/blockchain/Blockchain.h"

#include "veins/base/utils/Coord.h"
#include "veins/base/modules/BaseWorldUtility.h"

using namespace omnetpp;

namespace framework {

class MaliciousTransactionGenerator : public cSimpleModule, public TransactionGenerator {
protected:
    simtime_t timer;
    Blockchain* blockchain;
    AddressTable* addressTable;
    PositionTable* positionTable;
    veins::BaseWorldUtility* world;
    bool answerBack;
    int transactionsPerTurn;
    int range;

    virtual std::vector<std::shared_ptr<const Transaction>> makeTransactions() = 0;
    virtual Coord getPosition() = 0;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
public:
    virtual void handleRequest(NeighbourDiscovery* msg) override;
    virtual void handleReply(NeighbourReply* msg) override;
};

} /* namespace framework */

#endif /* SIMULATOR_MODULES_TRANSACTIONS_MALICIOUSTRANSACTIONGENERATOR_H_ */
