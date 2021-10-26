/*
 * BlockValidator.h
 *
 *  Created on: May 26, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_BLOCKVALIDATOR_H_
#define SIMULATOR_BLOCKCHAIN_BLOCKVALIDATOR_H_

#include <memory>

#include "BlockBuilder.h"
#include "Block.h"

namespace framework {

class BlockValidator {
public:
    // used to produce a valid block
    virtual void validate(BlockBuilder& builder) = 0;
    // used to check if a block is valid
    virtual bool validate(std::shared_ptr<const Block> block) = 0;
};

} /* namespace framework */

#endif /* SIMULATOR_BLOCKCHAIN_BLOCKVALIDATOR_H_ */
