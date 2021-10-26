/*
 * BlockLogger.h
 *
 *  Created on: Jun 22, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_UTILITY_BLOCKLOGGER_H_
#define SIMULATOR_UTILITY_BLOCKLOGGER_H_

#include <omnetpp.h>

#include "simulator/blockchain/Block.h"

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>

using namespace omnetpp;

namespace framework {

class BlockLogger : public cSimpleModule {
private:
    std::ofstream logFile;
    cOutVector totalReputationAvg;
    cOutVector type1ReputationAvg;
    cOutVector type2ReputationAvg;
    cOutVector numberOfNodes;
    cOutVector numberOfType1Nodes;
    cOutVector numberOfType2Nodes;
    cOutVector numberOfActiveNodes;
    cOutVector acceptableTransactions;
    cOutVector plausibleTransactions;
    cOutVector acceptedType2Transactions;
    cOutVector unacceptedType1Transactions;
    cOutVector totalTransactions;
    cOutVector type2Transactions;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage* msg);
    virtual void finish();
    void recordStatistics(const Block& block);

public:
    //BlockLogger(BlockLogger&& other) = default;
    void logBlock(std::shared_ptr<Block> block);
    void flush();
};

} /* namespace framework */

#endif /* SIMULATOR_UTILITY_BLOCKLOGGER_H_ */
