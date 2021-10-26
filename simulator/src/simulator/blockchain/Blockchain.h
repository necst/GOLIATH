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

#ifndef __SIMULATOR_BLOCKCHAIN_H_
#define __SIMULATOR_BLOCKCHAIN_H_

#include <list>
#include <memory>

#include <omnetpp.h>

#include "Block.h"
#include "BlockBuilder.h"
#include "NetworkView.h"
#include "BlockHarvester.h"
#include "BlockValidator.h"
#include "ConsensusModel.h"
#include "simulator/utility/BlockLogger.h"
#include "simulator/networking/AddressTable.h"
#include "simulator/networking/PositionTable.h"
#include "simulator/application/Application.h"

#include "veins/base/utils/SimpleAddress.h"

using namespace omnetpp;

namespace framework {

class NetworkView;
class ConsensusModel;
class BlockHarvester;

std::shared_ptr<Block> empty_block();

class Blockchain : public cSimpleModule, public cListener
{
protected:
    std::list<std::shared_ptr<Block>> blocks;
    LAddress::L3Type address;
    std::unique_ptr<BlockBuilder> currentBuilder;
    std::unique_ptr<BlockBuilder> nextBuilder;
    bool havePendingBlock;
    std::shared_ptr<Block> pendingBlock;
    int attempt = 0;
    int blocksize = 0;
    // cache the very last view to avoid too frequent recomputations
    std::unique_ptr<NetworkView> lastView;
    BlockLogger* logger;
    BlockHarvester* harvester;
    BlockValidator* validator;
    ConsensusModel* consensusModel;
    TransactionFilter* filter;
    AddressTable* addressTable;
    PositionTable* positionTable;
    Application* application;


    void requestBlockchainUpdate();
    virtual void prepareNextRound();
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) override;

public:
    Blockchain() {}
    virtual ~Blockchain() {};
    virtual int numInitStages() const override;

    // inserts the block in the local BC
    // obs: the block is logged by the harvester when it's ready
    // and not at insertion time in order to avoid concurrency control
    void insertBlock(std::shared_ptr<Block> block);
    std::shared_ptr<Block> getNthMostRecentBlock(size_t n);
    virtual void handleTransaction(std::shared_ptr<const Transaction> transaction, bool fromBroadcast = false);

    LAddress::L3Type getAddress() const;
    virtual BlockBuilder* getCurrentBuilder();
    virtual std::shared_ptr<Block> getPendingBlock();
    const NetworkView& getMostRecentView();
    const NetworkView& getAssociatedView(std::shared_ptr<Block> block);

    virtual void initializeData(Blockchain* source);
    const NetworkView& makeView(const Hash& hash, int attempt);

    virtual void finish(cComponent* component, simsignal_t signalID) override;
    const std::list<std::shared_ptr<Block>>& getBlocks() const;

private:
    // used to release the pointer to the oldest blocks
    // eventually, when all nodes have dropped it, it will leave main memory
    void dropOldBlocks();
};

}

#endif
