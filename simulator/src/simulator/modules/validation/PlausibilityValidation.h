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

#ifndef __SIMULATOR_PLAUSIBILITYVALIDATION_H_
#define __SIMULATOR_PLAUSIBILITYVALIDATION_H_

#include <omnetpp.h>
#include <unordered_set>

#include "simulator/blockchain/Blockchain.h"
#include "simulator/blockchain/BlockValidator.h"
#include "simulator/blockchain/RewardSystem.h"
#include "simulator/blockchain/TransactionInfo.h"
#include "ValidationResult.h"
#include "ValidationOptimizer.h"

using namespace omnetpp;

namespace framework {

class PlausibilityValidation : public cSimpleModule, public BlockValidator
{
public:
    using NodeSet = std::unordered_set<LAddress::L3Type>;
    using IndicatorMap = std::unordered_map<LAddress::L3Type, int>;
    using DetectionGraph = std::unordered_map<LAddress::L3Type, NodeSet>;
    using SubgraphResult = std::pair<IndicatorMap, std::vector<NodeSet>>;
    using NodePair = std::pair<LAddress::L3Type, LAddress::L3Type>;
protected:
    Blockchain* blockchain;
    RewardSystem* reward;
    ValidationOptimizer* optimizer;
    double maxSpeed;
    int margin;
    int depth;
    double threshold;
    int maxReputation;
    int maxConflicts;
    int maxSent;
    int minDecay;
    double decayRate;
    double varianceWeight;

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override;
    virtual void handleMessage(cMessage *msg) override;
    virtual ValidationResult validateBlockData(const BlockData& data,
            const StatusMap& oldStatuses,
            const std::vector<TransactionInfo>& blacklist);
    virtual void processValidationResult(const BlockData& data,
            ValidationResult& result);
    virtual void giveStartingReputation(const StatusMap& old, StatusMap& variations,
            const Stats& nodes, bool active);
    virtual std::vector<TransactionInfo> preprocessBlockData(const BlockData& data);

    virtual TransactionInfo validateTransaction(
            const std::vector<std::shared_ptr<Block>>& blocksToCheck,
            const TransactionPtr& transaction);

    virtual std::vector<LAddress::L3Type> blacklistMaliciousGroups(
            const std::vector<TransactionPtr>& transactions);

    virtual std::vector<int> countConflicts(
            const std::vector<TransactionPtr>& transactions,
            const IndicatorMap& belongsTo, int num);

    virtual SubgraphResult computeSubgraphs(const DetectionGraph& detectionGraph);

    virtual DetectionGraph createDetectionGraph(
            const std::vector<TransactionPtr>& transactions);

    virtual std::unordered_map<LAddress::L3Type, int> countConflicts(
            const std::vector<TransactionPtr>& transactions,
            const std::vector<TransactionInfo>& infos);

    virtual std::vector<NodePair> countConflicts(
            const std::vector<TransactionPtr>& transactions,
            const ValidationResult& result);

    virtual void updateStatuses(StatusMap& statuses,
            const ValidationResult& result);

    virtual std::vector<std::shared_ptr<Block>> getBlocksToCheck();

    virtual std::unordered_map<LAddress::L3Type, double> computeScaleFactors(const ValidationResult& result);

public:
    virtual void validate(BlockBuilder& builder) override;
    virtual bool validate(std::shared_ptr<const Block> block) override;
};
}
#endif
