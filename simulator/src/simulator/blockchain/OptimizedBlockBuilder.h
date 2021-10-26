/*
 * OptimizedBlockBuilder.h
 *
 *  Created on: Jul 31, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_OPTIMIZEDBLOCKBUILDER_H_
#define SIMULATOR_BLOCKCHAIN_OPTIMIZEDBLOCKBUILDER_H_

#include "BlockBuilder.h"
#include "OptimizedBuilderAllocator.h"

namespace framework {

class OptimizedBlockBuilder : public BlockBuilder {
protected:
    OptimizedBuilderAllocator* allocator;

    std::shared_ptr<BlockData> _sharedData;
    virtual BlockData* dataPtr() override;
    virtual const BlockData* const dataPtr() const override;
public:
    OptimizedBlockBuilder(int blocksize, TransactionFilter* filter, std::shared_ptr<BlockData> data);

    virtual bool addTransaction(std::shared_ptr<const Transaction> transaction) override;
    virtual std::shared_ptr<Block> build() override;
    virtual void restore(Block& block) override;
    virtual void take(BlockBuilder& builder) override;
};

} /* namespace framework */

#endif /* SIMULATOR_BLOCKCHAIN_OPTIMIZEDBLOCKBUILDER_H_ */
