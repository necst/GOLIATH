/*
 * BlockHarvester.h
 *
 *  Created on: May 26, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_BLOCKHARVESTER_H_
#define SIMULATOR_BLOCKCHAIN_BLOCKHARVESTER_H_

#include "NetworkView.h"

namespace framework {

class NetworkView;

class BlockHarvester {
public:
    virtual bool canHarvestBlock(const NetworkView& view) const = 0;
    virtual void reset() = 0;
};

} /* namespace framework */

#endif /* SIMULATOR_BLOCKCHAIN_BLOCKHARVESTER_H_ */
