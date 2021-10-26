/*
 * BlockLogger.cc
 *
 *  Created on: Jun 22, 2020
 *      Author: dvdmff
 */

#include "BlockLogger.h"

#include "simulator/networking/AddressTable.h"
#include "simulator/mobility/DoubleTypeTraCIManagerLaunchd.h"

#include <utility>

Define_Module(framework::BlockLogger);

namespace framework {

void BlockLogger::initialize() {
    totalReputationAvg.setName("TotalReputationAverage");
    type1ReputationAvg.setName("ActiveType1ReputationAverage");
    type2ReputationAvg.setName("ActiveType2ReputationAverage");
    numberOfNodes.setName("NumberOfNodes");
    numberOfType1Nodes.setName("NumberOfActiveType1Nodes");
    numberOfType2Nodes.setName("NumberOfActiveType2Nodes");
    numberOfActiveNodes.setName("NumberOfActiveNodes");
    acceptableTransactions.setName("NumberOfAcceptableTransactions");
    plausibleTransactions.setName("NumberOfPlausibleTransactions");
    acceptedType2Transactions.setName("NumberOfAcceptedType2Transactions");
    unacceptedType1Transactions.setName("NumberOfUnacceptedType1Transactions");
    totalTransactions.setName("TotalTransactions");
    type2Transactions.setName("Type2Transactions");

    logFile.open(par("logFile").stringValue());
}

void BlockLogger::handleMessage(cMessage* msg) {
    // nothing to do
}

void BlockLogger::finish() {
    if(logFile.is_open())
        logFile.close();
}

void BlockLogger::recordStatistics(const Block& block) {
    unsigned long long totalReputation = 0;
    int totalCount = 0;
    unsigned long long type1Reputation = 0;
    int type1Count = 0;
    unsigned long long type2Reputation = 0;
    int type2Count = 0;
    int activeNodes = 0;

    auto manager = dynamic_cast<DoubleTypeTraCIManagerLaunchd*>(getModuleByPath("<root>.manager"));
    auto table = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    ASSERT(table != nullptr);
    if(manager == nullptr) {
        // no type 2 vehicles
        for(auto& s: block.getAllStatuses()) {
            if(s.second.active) {
                totalReputation += s.second.reputation;
                activeNodes += 1;
            }
            ++totalCount;
        }
        type1Reputation = totalReputation;
        type1Count = activeNodes;
    } else {
        // there are 2 types of nodes
        for(auto& s: block.getAllStatuses()) {
            auto name = table->getNameFor(s.first);
            if(s.second.active) {
                totalReputation += s.second.reputation;
                activeNodes += 1;
                if(manager->isMainType(name)) {
                    type1Reputation += s.second.reputation;
                    ++type1Count;
                }
            }
            ++totalCount;
        }
        type2Reputation = totalReputation - type1Reputation;
        type2Count = activeNodes - type1Count;
    }

    unsigned acceptableTransactionsCount = 0;
    unsigned acceptedType2TransactionsCount = 0;
    unsigned unacceptedType1TransactionsCount = 0;
    unsigned plausibleTransactionsCount = 0;
    unsigned type2TransactionCount = 0;
    auto& infos = block.getAllInfo();
    auto& transations = block.getAllTransactions();
    for(auto i = 0; i < infos.size(); ++i) {
        if(infos[i].acceptable()) {
            ++acceptableTransactionsCount;
            auto mod = table->getNameFor(transations[i]->getSender());
            if(manager != nullptr && !manager->isMainType(mod))
                ++acceptedType2TransactionsCount;
        } else {
            auto mod = table->getNameFor(transations[i]->getSender());
            if(manager != nullptr && manager->isMainType(mod))
                ++unacceptedType1TransactionsCount;
        }
        if(infos[i].plausible())
            ++plausibleTransactionsCount;
        if(!manager->isMainType(table->getNameFor(transations[i]->getSender())))
            ++type2TransactionCount;
    }

    auto time = block.getCreationTime();
    totalReputationAvg.recordWithTimestamp(time, (totalCount == 0) ? 0 : totalReputation / (double)totalCount);
    type1ReputationAvg.recordWithTimestamp(time, (type1Count == 0) ? 0 : type1Reputation / (double)type1Count);
    type2ReputationAvg.recordWithTimestamp(time, (type2Count == 0) ? 0 : type2Reputation / (double)type2Count);
    numberOfNodes.recordWithTimestamp(time, totalCount);
    numberOfType1Nodes.recordWithTimestamp(time, type1Count);
    numberOfType2Nodes.recordWithTimestamp(time, type2Count);
    numberOfActiveNodes.recordWithTimestamp(time, activeNodes);
    acceptableTransactions.recordWithTimestamp(time, acceptableTransactionsCount);
    plausibleTransactions.recordWithTimestamp(time, plausibleTransactionsCount);
    acceptedType2Transactions.recordWithTimestamp(time, acceptedType2TransactionsCount);
    unacceptedType1Transactions.recordWithTimestamp(time, unacceptedType1TransactionsCount);
    totalTransactions.recordWithTimestamp(time, block.getTotalTransactions());
    type2Transactions.recordWithTimestamp(time, type2TransactionCount);
}

void BlockLogger::logBlock(std::shared_ptr<Block> block) {
    //avoid logging multiple times the same block
    if(!block->isLogged() && logFile.is_open()) {
        recordStatistics(*block);
        logFile << *block;
    }
    block->markAsLogged();
    // flush so that block gets actually logged. It may be slow,
    // but it ensures that data aren't lost if the program terminates
    // abnormally
    logFile.flush();
}

void BlockLogger::flush() {
    logFile.flush();
}

} /* namespace framework */
