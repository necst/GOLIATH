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

#include "OptimizedBuilderAllocator.h"
#include "OptimizedBlockBuilder.h"

Define_Module(framework::OptimizedBuilderAllocator);

namespace framework {


void OptimizedBuilderAllocator::initialize() {
    BuilderAllocator::initialize();
    mainData.clear();
    alternativeData.clear();
    oldestMainData = 0;
    oldestAlternativeData = 0;
    activeMainData = 0;
    activeAlternativeData = 0;
}

void OptimizedBuilderAllocator::finish() {
    BuilderAllocator::finish();
    // release ownership
    mainData.clear();
    alternativeData.clear();
}

std::shared_ptr<BlockBuilder> OptimizedBuilderAllocator::getBuilderForHeight(int height,
        int blocksize, TransactionFilter* filter) {

    if(mainData.size() <= height)
        mainData.resize(height + 1);

    // if there's no builder or there's an old one
    if(!mainData[height] || mainData[height].use_count() == 1) {
        mainData[height] = std::shared_ptr<BlockData>(new BlockData());
        activeMainData += (height > 3) ? 1 : 0;
    }

    oldestMainData = std::min(oldestMainData, std::max(height, 3));
    int idx = oldestMainData;
    int moveTo = -1;
    while(idx < mainData.size() && activeMainData > 3) {
        // keep allocated data relative to first 3 blocks because they're requested very often
        // due to new nodes joining
        if(idx > 3 && mainData[idx]) {
            if(mainData[idx].use_count() == 1 && idx != height) {
                mainData[idx].reset();
                --activeMainData;
            } else if(moveTo == -1)
                moveTo = idx - 1;
        }
        ++idx;
    }
    if(moveTo > 0)
        oldestMainData = moveTo;

    return std::shared_ptr<BlockBuilder>(
            new OptimizedBlockBuilder(blocksize, filter, mainData[height]));
}

std::shared_ptr<BlockBuilder> OptimizedBuilderAllocator::getAlternativeBuilderForHeight(
        int height, int blocksize, TransactionFilter* filter) {

    if(alternativeData.size() <= height)
        alternativeData.resize(height + 1);

    // if there's no builder or there's an old one
    if(!alternativeData[height] || alternativeData[height].use_count() == 1) {
        alternativeData[height] = std::shared_ptr<BlockData>(new BlockData());
        setTransactionCount(height, 0);
        activeAlternativeData += (height > 3) ? 1 : 0;
    }

    oldestAlternativeData = std::min(oldestAlternativeData, std::max(height, 3));
    int idx = oldestAlternativeData;
    int moveTo = -1;
    while(idx < alternativeData.size() && activeAlternativeData > 3) {
        // keep allocated data relative to first 3 blocks because they're requested very often
        // due to new nodes joining
        if(idx > 3 && alternativeData[idx]) {
            if(alternativeData[idx].use_count() == 1 && idx != height) {
                alternativeData[idx].reset();
                --activeAlternativeData;
            } else if(moveTo == -1)
                moveTo = idx - 1;
        }
        ++idx;
    }
    if(moveTo > 0)
        oldestAlternativeData = moveTo;

    return std::shared_ptr<BlockBuilder>(
            new OptimizedBlockBuilder(blocksize, filter, alternativeData[height]));
}

void OptimizedBuilderAllocator::incrementTransactionCount(int height) {
    if(totalTransactionCounts.size() <= height)
        totalTransactionCounts.resize(height + 1, 0);
    totalTransactionCounts[height] += 1;
}

unsigned long long OptimizedBuilderAllocator::getTransactionCount(int height) {
    if(totalTransactionCounts.size() <= height)
        return 0;
    return totalTransactionCounts[height];
}

void OptimizedBuilderAllocator::setTransactionCount(int height, unsigned long long count) {
    if(totalTransactionCounts.size() <= height)
        totalTransactionCounts.resize(height + 1, 0);
    totalTransactionCounts[height] = count;
}

}
