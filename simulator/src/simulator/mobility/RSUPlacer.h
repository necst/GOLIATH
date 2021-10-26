/*
 * RSUPlacer.h
 *
 *  Created on: Jul 24, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_MOBILITY_RSUPLACER_H_
#define SIMULATOR_MOBILITY_RSUPLACER_H_

#include <omnetpp.h>

#include <vector>
#include <memory>

#include "simulator/blockchain/Block.h"

using namespace omnetpp;

namespace framework {

class RSUPlacer {
public:
    virtual const std::vector<cModule*>& getRSUs() = 0;
    virtual std::shared_ptr<Block> getGenesisBlock() = 0;
};

}


#endif /* SIMULATOR_MOBILITY_RSUPLACER_H_ */
