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

#include <omp.h>
#include <algorithm>

#include "PlausibilityValidation.h"

#include "simulator/utility/functions.h"

Define_Module(framework::PlausibilityValidation);

#pragma omp declare reduction (merge: framework::StatusMap : framework::merge(omp_out, omp_in))
#pragma omp declare reduction (merge: framework::Stats : framework::merge(omp_out, omp_in))
#pragma omp declare reduction (concat: std::vector<framework::TransactionPtr> : framework::concat(omp_out, omp_in))
#pragma omp declare reduction (sum: std::vector<int> : framework::vectorSum(omp_out, omp_in))
#pragma omp declare reduction (merge: std::unordered_map<veins::LAddress::L3Type, int> : framework::merge(omp_out, omp_in))
#pragma omp declare reduction (merge: std::unordered_map<veins::LAddress::L3Type, std::set<veins::LAddress::L3Type>> : framework::merge(omp_out, omp_in))
#pragma omp declare reduction (concat: std::vector<framework::PlausibilityValidation::NodePair> : framework::concat(omp_out, omp_in))

namespace framework {
void PlausibilityValidation::initialize(int stage)
{
    if(stage != 1)
        return;
    blockchain = dynamic_cast<Blockchain*>(getModuleByPath("^.blockchain"));
    ASSERT(blockchain != nullptr);
    reward = dynamic_cast<RewardSystem*>(getModuleByPath("^.statusUpdater"));
    ASSERT(reward != nullptr);
    optimizer = dynamic_cast<ValidationOptimizer*>(getModuleByPath("<root>.validationOptimizer"));
    maxSpeed = par("maxSpeed").intValue() * 0.27778;
    margin = par("margin").intValue();
    depth = par("depth").intValue();
    threshold = par("threshold");
    maxReputation = reward->getMaxReward();
    maxConflicts = par("maxConflicts").intValue();
    maxSent = par("maxSent");
    minDecay = par("minDecay");
    decayRate = par("decayRate");
    varianceWeight = par("varianceWeight");
}

int PlausibilityValidation::numInitStages() const {
    return std::max(cSimpleModule::numInitStages(), 2);
}

void PlausibilityValidation::handleMessage(cMessage *msg)
{
    // nothing to do
}

TransactionInfo PlausibilityValidation::validateTransaction(
        const std::vector<std::shared_ptr<Block>>& blocksToCheck,
        const TransactionPtr& transaction)
{
    ASSERT(blocksToCheck.size() != 0);

    long long targetScore = 0;
    long long senderScore = 0;
    bool selfCoherent = true;

    auto& statuses = blocksToCheck[0]->getAllStatuses();
    auto position = transaction->getPosition();
    auto timestamp = transaction->getTimestamp();
    auto range = transaction->getRange();
    auto sender = transaction->getSender();
    auto target = transaction->getTarget();
    unsigned targetSenderCount = 0;
    unsigned targetTargetCount = 0;

    for(auto block: blocksToCheck) {
        auto& oldTransactions = block->getAllTransactions();
#pragma omp parallel for reduction(+:targetSenderCount) reduction(+:targetTargetCount) reduction(+:targetScore) reduction(+:senderScore) reduction(&&:selfCoherent)
        for(auto i = 0; i < oldTransactions.size(); ++i) {
            auto oldtIt = oldTransactions.begin() + i;
            auto oldInfo = block->getInfo(i);
            if(!selfCoherent || !oldInfo.plausible() || !oldInfo.acceptable())
                // skip old unreliable transactions
                continue;
            auto oldt = **oldtIt;
            if(oldt.getSender() == sender) {

                auto distance = oldt.getPosition().distance(position);
                auto delta = fabs((timestamp - oldt.getTimestamp()).dbl());
                auto bound = maxSpeed * delta;
                selfCoherent = selfCoherent && distance <= bound + margin;

            } else if(oldt.getTarget() == sender) {
                auto oldStatusIt = statuses.find(oldt.getSender());
                if(oldStatusIt != statuses.end()) {
                    auto distance = oldt.getPosition().distance(position);
                    auto delta = fabs((timestamp - oldt.getTimestamp()).dbl());
                    auto bound = maxSpeed * delta + oldt.getRange();
                    if(distance <= bound + margin)
                        senderScore+= oldStatusIt->second.reputation;
                    else
                        senderScore -= oldStatusIt->second.reputation;
                    ++targetSenderCount;
                }

            // prohibit targeting an inactive node
            } else if(oldt.getTarget() == target) {
                auto oldStatusIt = statuses.find(oldt.getSender());
                if(oldStatusIt != statuses.end() && oldStatusIt->second.active) {
                    auto distance = oldt.getPosition().distance(position);
                    auto delta = fabs((timestamp - oldt.getTimestamp()).dbl());
                    auto bound = maxSpeed * delta + range + oldt.getRange();
                    if(distance <= bound + margin)
                        targetScore += oldStatusIt->second.reputation;
                    else
                        targetScore -= oldStatusIt->second.reputation;
                    ++targetTargetCount;
                }
            }
        }
    }
    if(!selfCoherent)
        return TransactionInfo(false, false, "incoherent");

    unsigned count = targetSenderCount + targetTargetCount;
    if(count == 0)
        return TransactionInfo(true, false, "unsupported");
    double normalizedScore = (senderScore + targetScore) / (double)count;
    bool result = normalizedScore >= threshold;
    return TransactionInfo(result, result, "validation");
}

void PlausibilityValidation::giveStartingReputation(const StatusMap& old, StatusMap& variations,
        const Stats& nodes, bool active) {
    for(auto n: nodes) {
        //if node is new
        if(old.find(n.first) == old.end()) {
            variations[n.first] += reward->getStartingReputation();
            variations[n.first].active |= active;
        }
    }
}

ValidationResult PlausibilityValidation::validateBlockData(const BlockData& data,
        const StatusMap& oldStatuses,
        const std::vector<TransactionInfo>& infos)
{
    auto blocksToCheck = getBlocksToCheck();

    auto& transactions = data.transactions();
    auto outcomes = infos;
    Stats senders;
    Stats targets;
    Stats accepted;
    Stats rejected;
    AdjacencyMap adjacencyMap;


    // number of nodes is maximum if every node generates a transactions to a single other node
    // divide by 2 to keep symmetry into account
    size_t maxExpectedNumberOfNodes = transactions.size() >> 2;

#pragma omp parallel reduction(merge: senders, targets, accepted, rejected) reduction(merge: adjacencyMap)
    {
        // reserve space for the expected number of nodes for thread-local variables
        targets.max_load_factor(0.7);
        targets.reserve(maxExpectedNumberOfNodes);
        senders.max_load_factor(0.7);
        senders.reserve(maxExpectedNumberOfNodes);
        accepted.max_load_factor(0.7);
        accepted.reserve(maxExpectedNumberOfNodes);
        rejected.max_load_factor(0.7);
        rejected.reserve(maxExpectedNumberOfNodes);
        adjacencyMap.reserve(maxExpectedNumberOfNodes);
        adjacencyMap.max_load_factor(0.7);
#pragma omp for
        for(int i = 0; i < transactions.size(); ++i) {
            TransactionInfo outcome;
            if(!outcomes[i].valid()){
                outcome = validateTransaction(blocksToCheck, transactions[i]);
                outcomes[i] = outcome;
            }

            // register sender and receivers if they are not yet registered
            senders.insert({transactions[i]->getSender(), 0});
            targets.insert({transactions[i]->getTarget(), 0});

            if(outcome.plausible() && outcome.acceptable()) {
                accepted[transactions[i]->getSender()] += 1;
                // count the number of sent and received transaction
                senders[transactions[i]->getSender()] += 1;
                targets[transactions[i]->getTarget()] += 1;
                adjacencyMap[transactions[i]->getSender()].insert(transactions[i]->getTarget());
            } else if(!outcome.plausible())
                rejected[transactions[i]->getSender()] += 1;

        }
    }
    ValidationResult result;
    result.outcomes = std::move(outcomes);
    result.sent = std::move(senders);
    result.received = std::move(targets);
    result.accepted = std::move(accepted);
    result.rejected = std::move(rejected);
    result.adjacencyMap = std::move(adjacencyMap);
    return result;
}

void PlausibilityValidation::processValidationResult(const BlockData& data, ValidationResult& result) {
    std::vector<LAddress::L3Type> blacklist;

    for(auto& sender: result.accepted) {
        auto targetIt = result.received.find(sender.first);

        unsigned targetCount = (targetIt != result.received.end()) ? targetIt->second : 0;

        if(targetCount == 0 && sender.second > maxSent){
            // the node was targeted by too few nodes compared to the
            // number of transactions it has generated, so it's likely that the node
            // is not behaving properly
            blacklist.push_back(sender.first);
            // node has been blacklisted, so it can only lose reputation
            result.rejected[sender.first] += sender.second;
            sender.second = 0;
        }
    }
    //auto groupBlacklist = blacklistMaliciousGroups(data.transactions());
    //blacklist.insert(blacklist.end(), groupBlacklist.begin(), groupBlacklist.end());

    // sort blacklist vector to speed up lookup, as it will be performed lots of times
    std::sort(blacklist.begin(), blacklist.end());

    for(int i = 0; i < result.outcomes.size(); ++i) {
        auto sender = data.transactions()[i]->getSender();
        // check if sender is in blacklist through a binary search
        if(std::binary_search(blacklist.begin(), blacklist.end(), sender)) {
            if(result.outcomes[i].plausible())
                // transaction is implausible, correct validation results
                result.outcomes[i] = TransactionInfo(false, false, "blacklisted");
        }
    }

    auto conflicts = countConflicts(data.transactions(), result);
    Stats lostConflicts;

    for(auto c: conflicts) {
        auto reputation1 = 0;
        auto it = data.statuses().find(c.first);
        if(it != data.statuses().end())
            reputation1 = it->second.reputation;
        auto reputation2 = 0;
        it = data.statuses().find(c.second);
        if(it != data.statuses().end())
            reputation2 = it->second.reputation;
        auto loser = (reputation1 < reputation2) ? c.first : c.second;
        // add one penalty for each lost conflict
        result.rejected[loser] += 1;
        lostConflicts[loser] += 1;
    }
    // mark as unacceptable (but still plausible) transactions of nodes
    // that lost too many conflicts
    for(auto i = 0; i < data.transactions().size(); ++i) {
        auto& t = data.transactions()[i];
        auto it = lostConflicts.find(t->getSender());
        if(result.outcomes[i].acceptable() && it != lostConflicts.end() && it->second > maxConflicts) {
            result.outcomes[i] = TransactionInfo(true, false, "conflict");
            result.accepted[t->getSender()] -= 1;
        }
    }
}


PlausibilityValidation::DetectionGraph PlausibilityValidation::createDetectionGraph(
        const std::vector<TransactionPtr>& transactions) {

    DetectionGraph matrix;
    for(auto t: transactions) {
        matrix[t->getSender()].insert(t->getTarget());
        matrix[t->getTarget()].insert(t->getSender());
    }
    return matrix;
}

PlausibilityValidation::SubgraphResult PlausibilityValidation::computeSubgraphs(
        const DetectionGraph& detectionGraph){

    IndicatorMap belongsTo;
    std::vector<NodeSet> subgraphs;
    NodeSet toVisit;

    for(auto n: detectionGraph)
        toVisit.insert(n.first);

    while(toVisit.size() != 0) {
        subgraphs.push_back(NodeSet{});
        int pos = subgraphs.size() - 1;
        NodeSet next;
        next.insert(*toVisit.begin());
        while(next.size() != 0) {
            auto current = *next.begin();
            next.erase(current);
            subgraphs[pos].insert(current);
            belongsTo[current] = pos;
            auto detectionIt = detectionGraph.find(current);
            if(detectionIt != detectionGraph.end())
                for(auto n: detectionIt->second)
                    if(toVisit.find(n) != toVisit.end()) {
                        toVisit.erase(n);
                        next.insert(n);
                    }
        }
    }
    return std::make_pair(belongsTo, subgraphs);
}

std::vector<int> PlausibilityValidation::countConflicts(
        const std::vector<TransactionPtr>& transactions, const IndicatorMap& belongsTo, int num) {

    std::vector<int> matrix(num);

#pragma omp parallel for reduction(sum:matrix)
    for(auto i = 0; i < transactions.size(); ++i) {
        auto subgraph1 = belongsTo.at(transactions[i]->getSender());
#pragma omp parallel for reduction(sum:matrix)
        for(auto j = 0; j < transactions.size(); ++j) {
            auto subgraph2 = belongsTo.at(transactions[j]->getSender());
            if(subgraph1 != subgraph2) {
                auto dist = transactions[i]->getPosition().distance(transactions[j]->getPosition());
                auto deltaT = fabs(transactions[i]->getTimestamp().dbl() - transactions[j]->getTimestamp().dbl());
                auto range = std::max(transactions[i]->getRange(), transactions[j]->getRange());
                if(dist < range + margin && deltaT < 2) {
                    matrix[subgraph1] += 1;
                    //matrix[subgraph2] += 1;
                }
            }
        }
    }
    return matrix;
}
/*
std::vector<std::pair<LAddress::L3Type, LAddress::L3Type>> PlausibilityValidation::countConflicts(
        const std::vector<TransactionPtr>& transactions, const ValidationResult& result) {

    std::vector<std::pair<LAddress::L3Type, LAddress::L3Type>> conflicts;

#pragma omp parallel for reduction(sum:matrix)
    for(auto i = 0; i < transactions.size(); ++i) {
        if(!result.outcomes[i].plausible())
            continue;
        auto& t1 = transactions[i];
#pragma omp parallel for reduction(sum:matrix)
        for(auto j = 0; j < transactions.size(); ++j) {
            auto& t2 = transactions[j];
            // don't check the same transaction or invalid ones.
            // also, don't try find self-conflicts
            if(i == j || !result.outcomes[j].plausible() || t1->getSender() == t2->getSender())
                continue;
            auto dist = t1->getPosition().distance(t2->getPosition());
            auto deltaT = fabs(t1->getTimestamp().dbl() - t2->getTimestamp().dbl());
            auto range = std::max(t1->getRange(), t2->getRange());
            if(dist < range + margin && deltaT < 2) {
                conflicts.push_back(t1->getSender(), t2->getSender());
            }

        }
    }
    return conflicts;
}*/

std::vector<PlausibilityValidation::NodePair> PlausibilityValidation::countConflicts(
        const std::vector<TransactionPtr>& transactions, const ValidationResult& result) {

    std::vector<NodePair> conflicts;
    auto detectionGraph = createDetectionGraph(transactions);

    // compute all possible conflicts
#pragma omp parallel for reduction(concat:conflicts)
    for(auto i = 0; i < transactions.size(); ++i) {
        if(!result.outcomes[i].plausible())
            continue;
        auto& t1 = transactions[i];
#pragma omp parallel for reduction(concat:conflicts)
        for(auto j = 0; j < transactions.size(); ++j) {
            auto& t2 = transactions[j];
            // don't check invalid transactions or try find self-conflicts
            // skip all j <= i to avoid duplicates
            if(j <= i || !result.outcomes[j].plausible() || t1->getSender() == t2->getSender())
                continue;
            auto dist = t1->getPosition().distance(t2->getPosition());
            auto deltaT = fabs(t1->getTimestamp().dbl() - t2->getTimestamp().dbl());
            auto range = std::max(t1->getRange(), t2->getRange());
            auto x = std::min(t1->getSender(), t2->getSender());
            auto y = std::max(t1->getSender(), t2->getSender());
            auto& cut = detectionGraph[x];
            if(dist < range + margin && deltaT < 2 && cut.find(y) == cut.end()) {
                conflicts.push_back(std::make_pair(x, y));
            }

        }
    }
/*
    auto pairCmp = [] (NodePair p1, NodePair p2) {
        return p1.first < p2.first || (p1.first == p2.first && p1.second < p2.second);
    };
    std::sort(conflicts.begin(), conflicts.end(), pairCmp);
    std::vector<char> mask(conflicts.size(), 0);

    // remove all solved conflicts
#pragma omp parallel for
    for(auto i = 0; i < transactions.size(); ++i) {
        auto& t = transactions[i];
        auto x = std::min(t->getSender(), t->getTarget());
        auto y = std::max(t->getSender(), t->getTarget());
        auto pair = std::make_pair(x, y);
        auto lower = std::lower_bound(conflicts.begin(), conflicts.end(), pair, pairCmp);
        auto upper = std::upper_bound(conflicts.begin(), conflicts.end(), pair, pairCmp);
        if(lower != upper)
            mask[i] =
            //conflicts.erase(lower, upper);
    }*/
    return conflicts;
}

std::vector<LAddress::L3Type> PlausibilityValidation::blacklistMaliciousGroups(const std::vector<TransactionPtr>& transactions) {
    auto detectionGraph = createDetectionGraph(transactions);
    auto result = computeSubgraphs(detectionGraph);
    auto& belongsTo = result.first;
    auto& subgraphs = result.second;
    auto conflicts = countConflicts(transactions, belongsTo, subgraphs.size());

    std::vector<LAddress::L3Type> blacklistedNodes;
    for(int i = 0; i < subgraphs.size(); ++i) {
        if(conflicts[i] > fmax(10.0, 0.05 * subgraphs[i].size()))
            std::copy(subgraphs[i].begin(), subgraphs[i].end(), std::back_inserter(blacklistedNodes));
    }
    return blacklistedNodes;
}

std::unordered_map<LAddress::L3Type, int> PlausibilityValidation::countConflicts(
        const std::vector<TransactionPtr>& transactions,
        const std::vector<TransactionInfo>& infos) {


    DetectionGraph detectionMatrix;
    for(auto i = 0; i < transactions.size(); ++i) {
        if(infos[i].acceptable()) {
            auto& t = transactions[i];
            detectionMatrix[t->getSender()].insert(t->getTarget());
            detectionMatrix[t->getTarget()].insert(t->getSender());
        }
    }

    std::unordered_map<LAddress::L3Type, int> conflicts;

#pragma omp parallel for reduction(merge: conflicts)
    for(auto i = 0; i < transactions.size(); ++i) {
        if(!infos[i].acceptable())
            continue;
        auto sender1 = transactions[i]->getSender();
        auto position1 = transactions[i]->getPosition();
        auto range1 = transactions[i]->getRange();
        auto detectionSet = detectionMatrix[sender1];
#pragma omp parallel for reduction(merge: conflicts)
        for(auto j = 0; j < transactions.size(); ++j) {
            auto sender2 = transactions[j]->getSender();
            if(sender1 == sender2 || !infos[j].acceptable())
                continue;

            if(detectionSet.find(sender2) == detectionSet.end()) {
                auto dist = position1.distance(transactions[j]->getPosition());
                auto deltaT = fabs(transactions[i]->getTimestamp().dbl() - transactions[j]->getTimestamp().dbl());
                auto range = std::max(range1, transactions[j]->getRange());
                if(dist < range + margin && deltaT < 2) {
                    conflicts[sender1] += 1;
                }
            }
        }
    }
    return conflicts;
}

std::vector<TransactionInfo> PlausibilityValidation::preprocessBlockData(const BlockData& data) {
    auto size = data.transactions().size();
    std::vector<TransactionInfo> outcomes(size);

#pragma omp parallel for
    for(int j = 0; j < size; ++j) {
        auto& t1 = data.transactions()[j];
        if(t1->getSender() == t1->getTarget())
            outcomes[j] = TransactionInfo(false, false, "self-transaction");
        else {
            bool symmetric = false;
#pragma omp parallel for reduction(||:symmetric)
            for(int i = 0; i < size; ++i) {
                if(!symmetric) {
                    auto& t2 = data.transactions()[i];
                    bool found = t2->getSender() == t1->getTarget()
                        && t2->getTarget() == t1->getSender();
                    symmetric = symmetric || found;
                }
            }
            if(!symmetric)
                outcomes[j] = TransactionInfo(false, false, "unconfirmed");
        }
    }
    return outcomes;
}

std::vector<std::shared_ptr<Block>> PlausibilityValidation::getBlocksToCheck() {
    std::vector<std::shared_ptr<Block>> blocksToCheck;
    blocksToCheck.reserve(depth);
    auto highestBlock = blockchain->getNthMostRecentBlock(0);
    for(int i = 0; i < depth && i <= highestBlock->getHeight(); ++i)
        blocksToCheck.push_back(blockchain->getNthMostRecentBlock(i));
    return blocksToCheck;
}

std::unordered_map<LAddress::L3Type, double> PlausibilityValidation::computeScaleFactors(const ValidationResult& result) {
    auto blocksToCheck = getBlocksToCheck();
    while(blocksToCheck.size() > 2)
        blocksToCheck.pop_back();

    AdjacencyMap oldAdjacency;

    for(auto& b: blocksToCheck) {
        for(auto i = 0; i < b->getAllTransactions().size(); ++i) {
            if(b->getAllInfo()[i].acceptable() && b->getAllInfo()[i].plausible()) {
                auto& transaction = b->getAllTransactions()[i];
                oldAdjacency[transaction->getSender()].insert(transaction->getTarget());
            }
        }
    }

    std::unordered_map<LAddress::L3Type, double> scaleFactors;

    for(auto elem: result.adjacencyMap) {
        auto old = oldAdjacency.find(elem.first);
        auto invariance = 0.0;
        if(old != oldAdjacency.end()) {
            std::vector<LAddress::L3Type> intersection;
            std::set_intersection(elem.second.begin(),
                    elem.second.end(), old->second.begin(), old->second.end(),
                    std::back_inserter(intersection));
            invariance = intersection.size() / (double) elem.second.size();
        }
        auto variance = 1 - invariance * varianceWeight;
        scaleFactors[elem.first] = fmax(0, variance);
    }
    return scaleFactors;
}

void PlausibilityValidation::updateStatuses(StatusMap& statuses, const ValidationResult& result) {
    // deactivate all nodes
    for(auto& it: statuses)
        it.second.deactivate();

    auto startingRep = reward->getStartingReputation();
    for(auto n: result.sent) {
        // give starting reputation to new nodes
        if(statuses.find(n.first) == statuses.end())
            statuses[n.first] += startingRep;
        // a node is active if it has sent at least one transaction
        statuses[n.first].activate();
    }

    for(auto n: result.received) {
        // give starting reputation to new nodes
        if(statuses.find(n.first) == statuses.end())
            statuses[n.first] += startingRep;
    }

    auto scaleFactors = computeScaleFactors(result);

    // apply variations and decay over time
    for(auto& it: statuses) {
        auto acceptedIt = result.accepted.find(it.first);
        auto accepted = (acceptedIt != result.accepted.end()) ? acceptedIt->second : 0;
        // don't give more reputation for the same target more than once
        /*auto adjIt = result.adjacencyMap.find(it.first);
        if(adjIt != result.adjacencyMap.end() && adjIt->second.size() < accepted)
            accepted = adjIt->second.size();*/
        auto rejectedIt = result.rejected.find(it.first);
        auto rejected = (rejectedIt != result.rejected.end()) ? rejectedIt->second : 0;
        auto scaleIt = scaleFactors.find(it.first);
        auto scale = (scaleIt != scaleFactors.end()) ? scaleIt->second : 1;
        auto repReward = reward->getRewardFor(blockchain, it.first);
        auto repPenalty = reward->getPenaltyFor(blockchain, it.first);
        // compute variation
        int variation = 0;
        if(it.second.active)
            variation = scale * repReward * accepted - repPenalty * rejected;
        else
            variation = -fmax(minDecay, decayRate * it.second.reputation);
        it.second += variation;
        it.second.cap(maxReputation);
    }
}

void PlausibilityValidation::validate(BlockBuilder& builder) {
    builder.stopAddingTransactions();

    auto statuses = builder.getPreviousStatuses();
    auto& data = builder.data();

    // mark all as inactive, they will be enabled again during validation
    for(auto& it: statuses)
        it.second.deactivate();

    auto blacklist = preprocessBlockData(data);
    auto result = validateBlockData(data, statuses, blacklist);

    processValidationResult(data, result);

    if(optimizer != nullptr) {
        optimizer->cache(data.getHeight(), data.getAttempt(), std::move(result));
        result = optimizer->getResultFor(data.getHeight(), data.getAttempt());
    }

    updateStatuses(statuses, result);

    builder.setTransactionInfo(std::move(result.outcomes));
    builder.setStatuses(std::move(statuses));

    emit(registerSignal("blockValidated"), true);
}

bool PlausibilityValidation::validate(std::shared_ptr<const Block> block) {

    auto statuses = blockchain->getNthMostRecentBlock(0)->getAllStatuses();
    for(auto& it: statuses)
        it.second.deactivate();

    ValidationResult result;
    auto& data = block->getBlockData();

    if(optimizer == nullptr
            || !optimizer->hasResultFor(data.getHeight(), data.getAttempt())) {

        auto blacklist = preprocessBlockData(data);
        result = validateBlockData(data, statuses, blacklist);

        processValidationResult(data, result);

    }
    if(optimizer != nullptr) {
        EV_DEBUG << "optimization" << std::endl;
        if(!optimizer->hasResultFor(data.getHeight(), data.getAttempt()))
            optimizer->cache(data.getHeight(), data.getAttempt(), std::move(result));
        result = optimizer->getResultFor(data.getHeight(), data.getAttempt());
    }

    updateStatuses(statuses, result);

    bool outcomesEqual = result.outcomes.size() == block->getAllInfo().size()
            && std::equal(result.outcomes.begin(),
                    result.outcomes.end(),
                    block->getAllInfo().begin());
    bool statusesEqual = statuses.size() == block->getAllStatuses().size()
            && std::equal(statuses.begin(),
                    statuses.end(),
                    block->getAllStatuses().begin());

    return outcomesEqual && statusesEqual;
}

}
