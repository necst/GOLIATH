/*
 * OptimizedBlockBuilder.cc
 *
 *  Created on: Jul 31, 2020
 *      Author: dvdmff
 */

#include "OptimizedBlockBuilder.h"

namespace framework {

OptimizedBlockBuilder::OptimizedBlockBuilder(int blocksize, TransactionFilter* filter, std::shared_ptr<BlockData> data)
    : BlockBuilder(blocksize, filter),
      _sharedData {data}
{
    auto mod = cSimulation::getActiveSimulation()->getModuleByPath("<root>.allocator");
    allocator = dynamic_cast<OptimizedBuilderAllocator*>(mod);
}

bool OptimizedBlockBuilder::addTransaction(std::shared_ptr<const Transaction> transaction) {
    if(!refuseTransactions)
        allocator->incrementTransactionCount(dataPtr()->getHeight());
    return BlockBuilder::addTransaction(transaction);
}

std::shared_ptr<Block> OptimizedBlockBuilder::build() {
    ASSERT((bool)_data);
    auto dataCopy = std::unique_ptr<BlockData>(new BlockData(*_sharedData));
    // restore cap on statuses before finalizing
    for(auto it = dataCopy->statuses().begin();
            it != dataCopy->statuses().end();
            ++it)
        it->second.cap();
    return std::shared_ptr<Block>(new Block(blocksize, std::move(dataCopy),
            allocator->getTransactionCount(dataPtr()->getHeight())));
}

BlockData* OptimizedBlockBuilder::dataPtr() {
    return _sharedData.get();
}

const BlockData* const OptimizedBlockBuilder::dataPtr() const {
    return _sharedData.get();
}

void OptimizedBlockBuilder::restore(Block& block) {
    ASSERT(block.getHeight() != 0);
    _sharedData = std::move(block.data);
    refuseTransactions = false;
}

void OptimizedBlockBuilder::take(BlockBuilder& builder) {
    for(auto t: builder.data().transactions())
        addTransaction(t);
    // don't clear
}

} /* namespace framework */
