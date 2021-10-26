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

#ifndef __SIMULATOR_VALIDTRANSACTIONGENERATOR_H_
#define __SIMULATOR_VALIDTRANSACTIONGENERATOR_H_

#include <omnetpp.h>
#include <deque>

#include "simulator/blockchain/Blockchain.h"
#include "simulator/blockchain/TransactionGenerator.h"
#include "simulator/networking/PositionTable.h"

using namespace omnetpp;

namespace framework {
class ValidTransactionGenerator : public cSimpleModule, public TransactionGenerator
{
protected:
    simtime_t timer;
    Blockchain* blockchain;
    PositionTable* positionTable;
    //std::deque<std::shared_ptr<const Transaction>> buffer;
    //cMessage* syncMsg;
    cMessage* searchMsg;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    //void synchronize();
public:
    virtual void handleRequest(NeighbourDiscovery* msg) override;
    virtual void handleReply(NeighbourReply* msg) override;
};
}

#endif
