/*
 * BlockBuilder.cc
 *
 *  Created on: May 26, 2020
 *      Author: dvdmff
 */

#include <utility>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "BlockBuilder.h"
#include "simulator/utility/functions.h"

namespace framework {

BlockBuilder::BlockBuilder(int blocksize, TransactionFilter* filter)
    : blocksize {blocksize},
      _data {new BlockData()},
      filter {filter}
{
    // preallocate the right amount of memory
    _data->transactions().reserve(blocksize);
}

/*
 * Policy: ask the transaction filter how to insert the new transaction
 */
bool BlockBuilder::addTransaction(std::shared_ptr<const Transaction> transaction) {
    if (refuseTransactions)
        return false;

    totalTransactions += 1;
    auto data = dataPtr();
    //using trans_t = std::shared_ptr<const Transaction>;
    InsertionInfo insertionInfo = filter->filter(*data, *transaction);
    if(insertionInfo.overwrite) {
        if(insertionInfo.position >= data->transactions().size())
            data->transactions().resize(insertionInfo.position + 1);
        data->transactions()[insertionInfo.position] = transaction;


        return true;
    } else if (insertionInfo.insert) {
        ASSERT(data->transactions().size() < blocksize);
        data->transactions().push_back(transaction);
        // don't waste memory
        if(data->transactions().size() == blocksize && data->transactions().capacity() > blocksize)
            data->transactions().shrink_to_fit();
        return true;
    }
    return false;
}

const StatusMap& BlockBuilder::getPreviousStatuses() const {
    return previousBlock->getAllStatuses();
}

void BlockBuilder::setStatuses(StatusMap&& statuses) {
    dataPtr()->statuses() = statuses;
}

void BlockBuilder::updateStatus(const LAddress::L3Type& id, const NodeStatus& status) {
    auto it = dataPtr()->statuses().find(id);
    if(it == dataPtr()->statuses().end())
        dataPtr()->statuses()[id] = status;
    else
        it->second.combine(status);
}

void BlockBuilder::setPreviousBlock(std::shared_ptr<Block> previousBlock) {
    dataPtr()->previousHash(previousBlock->getHash());
    this->previousBlock = previousBlock;
    dataPtr()->setHeight(previousBlock->getHeight() + 1);
}

void BlockBuilder::stopAddingTransactions() {
    ASSERT(dataPtr() != nullptr);
    dataPtr()->transactionInfo() = std::vector<TransactionInfo>(
            dataPtr()->transactions().size());
    refuseTransactions = true;
}

void BlockBuilder::setTransactionInfo(size_t transactionIndex, const TransactionInfo& info) {
    dataPtr()->transactionInfo()[transactionIndex] = info;
}

void BlockBuilder::setTransactionInfo(std::vector<TransactionInfo>&& infos) {
    dataPtr()->transactionInfo() = infos;
}

std::shared_ptr<Block> BlockBuilder::build() {
    // restore cap on statuses before finalizing
    for(auto it = dataPtr()->statuses().begin();
            it != dataPtr()->statuses().end();
            ++it)
        it->second.cap();
    return std::shared_ptr<Block>(new Block(blocksize, std::move(_data), totalTransactions));
}

int BlockBuilder::getHeight() const {
    return dataPtr()->getHeight();
}

void BlockBuilder::setHeight(int height) {
    dataPtr()->setHeight(height);
}

void BlockBuilder::setAttempt(int attempt) {
    dataPtr()->setAttempt(attempt);
}

void BlockBuilder::prepare() {
    // copy node statuses only when explicitly needed (i.e. prior to validation)
    // this avoid wasting memory for blocks that will never be used
    dataPtr()->statuses() = previousBlock->getAllStatuses();
    // release cap on statuses to allow validation
    for(auto it = dataPtr()->statuses().begin();
            it != dataPtr()->statuses().end();
            ++it)
        it->second.uncap();
}

const BlockData& BlockBuilder::data() const {
    return *dataPtr();
}

bool BlockBuilder::isFrozen() const {
    return refuseTransactions;
}

void BlockBuilder::restore(Block& block) {
    ASSERT(block.height != 0);
    _data = std::move(block.data);
    refuseTransactions = false;
}

void BlockBuilder::take(BlockBuilder& builder) {
    for(auto t: builder.dataPtr()->transactions()) {
        addTransaction(t);
    }
    builder.dataPtr()->transactions().clear();
}

void BlockBuilder::fillUpWith(std::shared_ptr<Block> olderBlock) {
    for(auto t = olderBlock->getAllTransactions().rbegin();
            t != olderBlock->getAllTransactions().rend();
            ++t)
        addTransaction(*t);
}

BlockData* BlockBuilder::dataPtr() {
    return _data.get();
}

const BlockData* const BlockBuilder::dataPtr() const {
    return _data.get();
}

} /* namespace framework */
