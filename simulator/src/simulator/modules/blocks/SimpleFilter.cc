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

#include "SimpleFilter.h"

#include "simulator/utility/functions.h"

#include <map>
#include <iterator>
#include <algorithm>
#include <omp.h>

Define_Module(framework::SimpleFilter);

#pragma omp declare reduction (merge: std::map<LAddress::L3Type, long>: framework::merge(omp_out, omp_in))

namespace framework {
void SimpleFilter::initialize()
{
    // do nothing
}

void SimpleFilter::handleMessage(cMessage *msg)
{
    // do nothing
}

InsertionInfo SimpleFilter::filter(const BlockData& currentData, const Transaction& transaction) {
    if(currentData.transactions().size() < getParentModule()->par("blocksize").intValue())
        //there's still space, just accept transaction
        return InsertionInfo(true);

    // must discard one transaction (either this one or an older one)
    // strategy: remove oldest, break ties using order of arrival

    int randIdx = getRNG(0)->intRand(currentData.transactions().size());

    if(transaction.getTimestamp() > currentData.transactions()[randIdx]->getTimestamp())
        return InsertionInfo(true, randIdx);

    return InsertionInfo(false);
/*
    // compute stats and choose candidate for elimination
    std::map<LAddress::L3Type, long> counts;

//#pragma omp parallel for reduction(merge: counts)
    for(int i = 0; i < currentData.transactions().size(); ++i){
        auto t = currentData.transactions()[i];
        auto it = counts.find(t->getSender());
        if(it == counts.end())
            counts[t->getSender()] = 1;
        else
            counts[it->first] += 1;
    }

    using entry_t = std::map<LAddress::L3Type, long>::value_type;
    auto highestCount = framework::max_element(
            counts.begin(),
            counts.end(),
            [] (entry_t a, entry_t b) {
                return a.second < b.second
                        || (a.second == b.second && a.first < b.first);
            }
        )->first;

    auto oldestIt = currentData.transactions().begin();
    // since transactions are ordered, just scroll the vector until
    // the first transaction with the correct sender is found
    while(oldestIt != currentData.transactions().end()
            && (*oldestIt)->getSender() != highestCount
            && (*oldestIt)->getTarget() != highestCount)
        ++oldestIt;
    // a result must be found otherwise there's an error
    ASSERT(oldestIt != currentData.transactions().end());

    auto oldest = *oldestIt;
    // this check should be on top
    if(oldest->getTimestamp() > transaction.getTimestamp()
            || oldestIt == currentData.transactions().end())
        // either new transaction is too old or it wans't possible to find an transaction to eliminate
        return InsertionInfo(false);

    return InsertionInfo(true, std::distance(currentData.transactions().begin(), oldestIt));*/
}
/*
std::vector<InsertionInfo> SimpleFilter::filter(const BlockData& currentData,
        const std::vector<std::shared_ptr<const Transaction>>& transactions) {

    std::vector<InsertionInfo> infos(transactions.size(), {false});
    int insertedCount = 0;
    int blocksize = getParentModule()->par("blocksize").intValue();

    while(currentData.transactions().size() + insertedCount < blocksize) {
        //there's still space, just accept transaction
        infos[insertedCount] = InsertionInfo(true);
        ++insertedCount;
    }

    // must discard some transactions
    // strategy: remove oldest, break ties using node addresses

    // compute stats and choose candidate for elimination
    std::map<LAddress::L3Type, long> counts;

    for(int i = 0; i < currentData.transactions().size(); ++i){
        auto t = currentData.transactions()[i];
        auto it = counts.find(t->getSender());
        if(it == counts.end())
            counts[t->getSender()] = 1;
        else
            counts[it->first] += 1;
    }

    using entry_t = std::map<LAddress::L3Type, long>::value_type;
    auto highestCounts = framework::max_n_element(
        counts.begin(),
        counts.end(),
        transactions.size() - insertedCount,
        [] (entry_t a, entry_t b) {
            return a.second < b.second
                    || (a.second == b.second && a.first < b.first);
        }
    );

    using iter = decltype(*highestCounts.begin());

    std::vector<decltype(currentData.transactions().begin())> skip;

    for(int i = insertedCount; i < transactions.size(); ++i) {
        auto transaction = transactions[i];

        auto highestCount = *framework::max_element(
            highestCounts.begin(),
            highestCounts.end(),
            [] (iter a, iter b) {
                return a->second < b->second
                        || (a->second == b->second && a->first < b->first);
            }
        );

        // consider only first quarter of transactions
        int ignore = uniform(0, highestCount->second >> 2);

        auto oldestIt = currentData.transactions().begin();

        // since transactions are ordered, just scroll the vector until
        // the first transaction with the correct sender is found
        while((oldestIt != currentData.transactions().end()
                && (ignore > 0 || (*oldestIt)->getSender() != highestCount->first))
                || std::find(skip.begin(), skip.end(), oldestIt) != skip.end()) {
            ++oldestIt;
            if((*oldestIt)->getSender() == highestCount->first)
                --ignore;
        }

        auto oldest = *oldestIt;

        if((oldest->getTimestamp() <= transaction->getTimestamp() && oldest->getSender() != transaction->getSender())
                && (oldest->getTimestamp() < transaction->getTimestamp() && oldest->getSender() == transaction->getSender())
                && oldestIt != currentData.transactions().end()) {
            // either new transaction is too old or it wans't possible to find an transaction to eliminate
            infos[i] = InsertionInfo(true, std::distance(currentData.transactions().begin(), oldestIt));
            skip.push_back(oldestIt);
        }
    }
    return infos;
}*/

}
