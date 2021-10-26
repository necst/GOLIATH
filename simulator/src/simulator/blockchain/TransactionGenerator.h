/*
 * TransactionGenerator.h
 *
 *  Created on: Jul 4, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_TRANSACTIONGENERATOR_H_
#define SIMULATOR_BLOCKCHAIN_TRANSACTIONGENERATOR_H_

#include "simulator/messages/NeighbourDiscovery_m.h"
#include "simulator/messages/NeighbourReply_m.h"

namespace framework {
class TransactionGenerator {
public:
    virtual void handleRequest(NeighbourDiscovery* msg) = 0;
    virtual void handleReply(NeighbourReply* msg) = 0;
};
}


#endif /* SIMULATOR_BLOCKCHAIN_TRANSACTIONGENERATOR_H_ */
