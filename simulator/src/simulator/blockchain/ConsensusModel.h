/*
 * ConsensusModel.h
 *
 *  Created on: May 26, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_CONSENSUSMODEL_H_
#define SIMULATOR_BLOCKCHAIN_CONSENSUSMODEL_H_

#include <memory>
#include <vector>
#include <omnetpp.h>

#include "veins/base/utils/SimpleAddress.h"

#include "NetworkView.h"
#include "Block.h"

using namespace veins;

namespace framework {

class NetworkView;

/*
 * Defines the sequence of operations to perform in order to achieve consensus
 * Harvesters are those that initiate the protocol by invoking onHarvest
 * The other will then react to the messages
 * This component must be stateful (?) in order to react properly
 */
class ConsensusModel {
public:
    // what to do on harvesting
    virtual void onHarvest(const NetworkView& view, std::shared_ptr<Block> block) = 0;
    // how to react to messages
    virtual void onMessage(const NetworkView& view, omnetpp::cMessage* msg) = 0;
    // verifies that the block has been approved according to the consensus model rules
    // depending on the protocol it may also invoke
    // obs: consensus fails no block is valid
    virtual bool verify(const NetworkView& view, std::shared_ptr<const Block> block) = 0;
    // Generate the network view from parameters
    virtual std::unique_ptr<NetworkView> generateView(Hash seed, int attempt) = 0;
};

} /* namespace framework */

#endif /* SIMULATOR_BLOCKCHAIN_CONSENSUSMODEL_H_ */
