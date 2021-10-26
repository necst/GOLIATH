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

#ifndef __SIMULATOR_OPTIMIZEDBLOCKCHAIN_H_
#define __SIMULATOR_OPTIMIZEDBLOCKCHAIN_H_

#include <omnetpp.h>

#include "Blockchain.h"
#include "BuilderAllocator.h"

using namespace omnetpp;

namespace framework {
class OptimizedBlockchain : public Blockchain
{
protected:
    BuilderAllocator* allocator;
    std::shared_ptr<BlockBuilder> currentBuilder;
    std::shared_ptr<BlockBuilder> nextBuilder;
    bool useMainBuilder;

    virtual void prepareNextRound() override;
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) override;
    void updateBuilders(int height);

public:
    virtual void handleTransaction(std::shared_ptr<const Transaction> transaction,
            bool fromBroadcast = false) override;
    virtual BlockBuilder* getCurrentBuilder() override;
    virtual std::shared_ptr<Block> getPendingBlock() override;
    virtual void initializeData(Blockchain* source) override;
};
}
#endif
