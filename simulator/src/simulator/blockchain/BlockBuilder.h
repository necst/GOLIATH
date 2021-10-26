/*
 * BlockBuilder.h
 *
 *  Created on: May 26, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_BLOCKBUILDER_H_
#define SIMULATOR_BLOCKCHAIN_BLOCKBUILDER_H_

#include <memory>

#include "veins/base/utils/SimpleAddress.h"
#include "Transaction.h"
#include "NodeStatus.h"
#include "Block.h"
#include "TransactionFilter.h"

using namespace veins;

namespace framework {

class BlockBuilder {
protected:
    int blocksize;
    bool refuseTransactions = false;
    std::unique_ptr<BlockData> _data;
    TransactionFilter* filter;
    std::shared_ptr<Block> previousBlock;
    unsigned long long totalTransactions = 0;

    virtual BlockData* dataPtr();
    virtual const BlockData* const dataPtr() const;
public:
    BlockBuilder(int blocksize, TransactionFilter* filter);
    virtual ~BlockBuilder() {}

    virtual bool addTransaction(std::shared_ptr<const Transaction> transaction);
    const BlockData& data() const;
    void updateStatus(const LAddress::L3Type& id, const NodeStatus& status);
    const StatusMap& getPreviousStatuses() const;
    void setStatuses(StatusMap&& statuses);

    void setPreviousBlock(std::shared_ptr<Block> previousBlock);
    void setAttempt(int attempt);
    int getHeight() const;
    void setHeight(int height);
    void stopAddingTransactions();
    // requires stopAddingTransactions() to be called first
    // may throw out-of-bounds exception
    void setTransactionInfo(size_t transactionIndex, const TransactionInfo& info);
    void setTransactionInfo(std::vector<TransactionInfo>&& infos);
    void prepare();
    virtual std::shared_ptr<Block> build();
    bool isFrozen() const;
    virtual void restore(Block& block);
    virtual void take(BlockBuilder& builder);
    void fillUpWith(std::shared_ptr<Block> olderBlock);
};

} /* namespace framework */

#endif /* SIMULATOR_BLOCKCHAIN_BLOCKBUILDER_H_ */
