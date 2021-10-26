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

#include "MimicTransactionGenerator.h"
#include "simulator/utility/functions.h"

#include <algorithm>

Define_Module(framework::MimicTransactionGenerator);

namespace framework {

void MimicTransactionGenerator::initialize() {
    MaliciousTransactionGenerator::initialize();
    target = LAddress::L3BROADCAST();
    waitTimer = new cMessage();
    generateTransactions = true;
}

void MimicTransactionGenerator::finish() {
    MaliciousTransactionGenerator::finish();
    cancelAndDelete(waitTimer);
}

void MimicTransactionGenerator::handleMessage(cMessage* msg) {
    if(msg == waitTimer)
        generateTransactions = true;
    else
        MaliciousTransactionGenerator::handleMessage(msg);
}

bool MimicTransactionGenerator::isActive(std::shared_ptr<Block> block,
        LAddress::L3Type target)
{
    auto it = block->getAllStatuses().find(target);
    if(it == block->getAllStatuses().end())
        return false;
    return it->second.active;
}

bool MimicTransactionGenerator::pickNewTarget() {
    // pick a new target
    auto block = blockchain->getNthMostRecentBlock(0);
    std::vector<LAddress::L3Type> addresses;
    int maxAttempts = 5;
    auto& transactions = block->getAllTransactions();
    auto& infos = block->getAllInfo();
    auto& statuses = block->getAllStatuses();
    if(transactions.size() == 0)
        return false;
    int idx = getRNG(0)->intRand(transactions.size());
    auto sender = transactions[idx]->getSender();
    auto statusIt = statuses.find(sender);
    while(maxAttempts > 0 &&
            !(sender != blockchain->getAddress() && statusIt != statuses.end()
                    && statusIt->second.active && infos[idx].acceptable())) {
        idx = getRNG(0)->intRand(transactions.size());
        sender = transactions[idx]->getSender();
        statusIt = statuses.find(sender);
        --maxAttempts;
    }

    if(sender != blockchain->getAddress() && statusIt != statuses.end()
            && statusIt->second.active && infos[idx].acceptable()) {
        target = sender;
        EV_DEBUG << "Changed target to " << target << " at " << simTime() << std::endl;
        return true;
    }
    return false;
}

bool MimicTransactionGenerator::isTargetInvalid() {
    auto block = blockchain->getNthMostRecentBlock(0);
    return target == LAddress::L3BROADCAST()
        || (/*hashEqual(lastBlock, block->getHash()) || */!isActive(block, target));
}

std::vector<std::shared_ptr<const Transaction>> MimicTransactionGenerator::makeTransactions() {
    // if there's no target or the block has changed and the previous
    // target is now inactive
    if(isTargetInvalid()) {
        auto hasTarget = pickNewTarget();
        generateTransactions = false;
        if(hasTarget && !waitTimer->isScheduled()) {
            auto blocktime = SimTime(
                    getModuleByPath("^.blockchain")->par("blocktime"),
                    SimTimeUnit::SIMTIME_S);
            scheduleAt(simTime() + blocktime * 3, waitTimer);
        }
    }

    std::vector<std::shared_ptr<const Transaction>> generatedTransactions;
    generatedTransactions.reserve(transactionsPerTurn);

    if(generateTransactions) {
        auto& transactions = blockchain->getCurrentBuilder()->data().transactions();
        using trans_t = std::shared_ptr<const Transaction>;
        std::vector<trans_t> trs;
        //std::shared_ptr<const Transaction> targetTransaction;
        // pick most recent transaction from targeted node to replay
        for(auto& t: transactions){
            if(t->getSender() == target){
                if (simTime() - t->getTimestamp() <= timer) {
                    generatedTransactions.push_back(
                            std::shared_ptr<const Transaction>(new Transaction(
                                blockchain->getAddress(),
                                t->getTarget(),
                                t->getPosition(),
                                t->getRange(),
                                simTime())));
                    if(generatedTransactions.size() == transactionsPerTurn)
                        break;
                }
            }
        }
        if(generatedTransactions.size() > 0)
            lastTargetPosition = generatedTransactions[0]->getPosition();
    }
    return generatedTransactions;
}

Coord MimicTransactionGenerator::getPosition() {
    return lastTargetPosition;
}

}
