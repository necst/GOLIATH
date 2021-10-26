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

#include "Blockchain.h"
#include "simulator/application/Application.h"
#include "simulator/utility/BlockLogger.h"
#include "simulator/utility/Vehicle.h"
#include "simulator/utility/functions.h"
#include "simulator/networking/AddressTable.h"
#include "simulator/networking/PositionTable.h"
#include "simulator/blockchain/ConsensusModel.h"
#include "simulator/messages/DownloadBlockchainRequest_m.h"
#include "simulator/messages/TransactionBroadcast_m.h"
#include "simulator/mobility/RSUPlacer.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/base/utils/FindModule.h"

#include <iterator>
#include <stdexcept>

Define_Module(framework::Blockchain);

using namespace veins;

namespace framework {

std::shared_ptr<Block> empty_block() {
    static std::shared_ptr<Block> block;
    if(!block){
        std::unique_ptr<BlockData> data(new BlockData());
        data->statuses()[0L] = NodeStatus(1, true, 0);
        data->setHeight(0);
        block = std::shared_ptr<Block>(new Block(0, std::move(data), 0));
        block->signBlock(0L);
    }
    return block;
}

int Blockchain::numInitStages() const {
    return std::max(cSimpleModule::numInitStages(), 2);
}

void Blockchain::initialize(int stage) {

    cSimpleModule::initialize(stage);
    // wait for other components to be initialized to avoid race conditions
    if(stage != 1)
        return;

    // gather pointers to the submodules
    filter = dynamic_cast<TransactionFilter*>(getModuleByPath("^.filter"));
            //getParentModule()->getSubmodule("filter"));
    ASSERT(filter != nullptr);
    logger = dynamic_cast<BlockLogger*>(getModuleByPath("<root>.logger"));
    ASSERT(logger != nullptr);
    harvester = dynamic_cast<BlockHarvester*>(getModuleByPath("^.blockHarvester"));
            //getParentModule()->getSubmodule("blockHarvester"));
    ASSERT(harvester != nullptr);
    consensusModel = dynamic_cast<ConsensusModel*>(getModuleByPath("^.consensusModel"));
            //getParentModule()->getSubmodule("consensusModel"));
    ASSERT(consensusModel != nullptr);
    validator = dynamic_cast<BlockValidator*>(getModuleByPath("^.blockValidator"));
            //getParentModule()->getSubmodule("blockValidator"));
    ASSERT(validator != nullptr);
    auto app = FindModule<>::findHost(this)->getModuleByPath(".app.application");
    //app = app->getSubmodule("app")->getSubmodule("application");
    application = dynamic_cast<Application*>(app);
    ASSERT(application != nullptr);
    addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    ASSERT(addressTable != nullptr);
    positionTable = dynamic_cast<PositionTable*>(getModuleByPath("<root>.positionTable"));
    ASSERT(positionTable != nullptr);

    blocksize = par("blocksize");
    // prepare current and next block builders
    currentBuilder = std::unique_ptr<BlockBuilder>(new BlockBuilder(blocksize, filter));
    nextBuilder = std::unique_ptr<BlockBuilder>(new BlockBuilder(blocksize, filter));
    currentBuilder->setHeight(1);
    auto vehicle = FindModule<>::findHost(this);
    addressTable->registerVehicle(vehicle->getFullPath(), vehicle);
    address = addressTable->getAddress(vehicle->getFullPath());


    FindModule<>::findHost(this)->subscribe("consensusTerminated", this);
    FindModule<>::findHost(this)->subscribe("blockValidated", this);
    FindModule<>::findHost(this)->subscribe("harvestSignal", this);

    if(par("downloadAtStart").boolValue()) {
        //do this only for non-prejoined nodes
        // start with genesis block, so that the node always have at least one block
        auto rsuPlacer = dynamic_cast<RSUPlacer*>(getModuleByPath("<root>.rsuPlacer"));
        insertBlock(rsuPlacer->getGenesisBlock());
        // request an update so the node doesn't have to wait until a failed block insertion
        requestBlockchainUpdate();
    }
}

void Blockchain::finish() {
    // remove references to this node from address table
    addressTable->leave(address);
    // remove node position from position table to avoid detection
    positionTable->leave(address);
    if(currentBuilder) {
        auto ptr = currentBuilder.release();
        delete ptr;
    }
    if(nextBuilder) {
        auto ptr = nextBuilder.release();
        delete ptr;
    }

    EV_DEBUG << "terminating with blockchain at height "
            << getNthMostRecentBlock(0)->getHeight() << std::endl;
}

void Blockchain::finish(cComponent* component, simsignal_t signalID) {
    cListener::finish(component, signalID);
}

void Blockchain::handleMessage(cMessage *msg)
{
    // nothing to do
}

void Blockchain::receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) {
    if(signalID == registerSignal("consensusTerminated")) {
        // nothing to do
    } else if(signalID == registerSignal("blockValidated")) {
        // after validation, ask the consensus model what to do
        consensusModel->onHarvest(getAssociatedView(getPendingBlock()), getPendingBlock());
    } else if(signalID == registerSignal("harvestSignal")) {
        if(b) {
            // if this node is an harvester
            if(havePendingBlock) {
                // last validation on this node failed, restore the block builder with current data
                // and proceed as in the first time
                havePendingBlock = false;
                currentBuilder->restore(*pendingBlock);
                // drop pointer to pending block as it's invalid now
                pendingBlock.reset();
                // take transactions that were going to be added in next block
                currentBuilder->take(*nextBuilder);
            }
            currentBuilder->setAttempt(attempt);
            // increase attempt before validation, otherwise the increase may
            // overwrite the newly set attempt number
            attempt += 1;
            // keep blocks with maximum information content
            if(getCurrentBuilder()->data().transactions().size() < blocksize / 2)
                currentBuilder->fillUpWith(getNthMostRecentBlock(0));

            // if this node is the harvester, start the validation procedure
            validator->validate(*getCurrentBuilder());
        } else
            // just update the view in case the block gets rejected
            attempt += 1;
    }
}

void Blockchain::prepareNextRound() {
    attempt = 0;
    // shift builders
    currentBuilder = std::move(nextBuilder);
    currentBuilder->setAttempt(0);
    nextBuilder = std::unique_ptr<BlockBuilder>(new BlockBuilder(blocksize, filter));
    currentBuilder->setPreviousBlock(getNthMostRecentBlock(0));
    havePendingBlock = false;
    // drop pointer to allow block to be freed through RAII
    pendingBlock.reset();
    // don't reset harvester for genesis block insertion
    //if(getNthMostRecentBlock(0)->getHeight() != 0)
    harvester->reset();
    // reset may require a context change
    Enter_Method_Silent();
}

void Blockchain::insertBlock(std::shared_ptr<Block> block) {
    ASSERT(block->hasData());
    if(blocks.size() > 0 && consensusModel->verify(getAssociatedView(block), block)
            && hashEqual(blocks.front()->getHash(), block->getPreviousHash())){
        EV_DEBUG << "inserting block " << block->getHeight() << std::endl;
        // block has been verified according to the consensus model, so it can be
        // trusted
        blocks.push_front(block);
        prepareNextRound();
        logger->logBlock(block);
        EV_DEBUG << "block " << block->getHeight() << " inserted (now attempt is" << std::endl;
    } else if(blocks.size() == 0 && block->getHeight() == 0) {
        EV_DEBUG << "inserting genesis block " << std::endl;
        // insert genesis block
        blocks.push_front(block);
        prepareNextRound();
        logger->logBlock(block);
    }
    else if (getNthMostRecentBlock(0)->getHeight() < block->getHeight() - 1) {
        // Block may be valid, but it's impossible to determine it as its height
        // in the blockchain is higher than the highest heigth of the locally stored blocks.
        // Send a request to download the most recent blocks and drop current block as
        // it will likely be included on the new blockchain
        EV_DEBUG << "can't insert block, request update" << std::endl;
        requestBlockchainUpdate();
    }
    // block is simply invalid or too late, drop it
    EV_DEBUG << "there are now " << blocks.size() << " blocks" << std::endl;
    // drop old blocks to keep only the specified number of blocks in memory
    dropOldBlocks();
}

std::shared_ptr<Block> Blockchain::getNthMostRecentBlock(size_t n) {
    if(n >= blocks.size())
        return std::shared_ptr<Block> {};

    auto it = blocks.begin();
    std::advance(it, n);
    return *it;
}

void Blockchain::dropOldBlocks() {
    if(par("storedChainLength").intValue() < 0)
        // keep all blocks
        return;
    while(blocks.size() > par("storedChainLength").intValue()) {
        // drop last item
        blocks.pop_back();
    }
}

void Blockchain::handleTransaction(std::shared_ptr<const Transaction> transaction, bool fromBroadcast) {
    bool inserted = false;
    if(currentBuilder->isFrozen())
        inserted = nextBuilder->addTransaction(transaction);
    else
        inserted = currentBuilder->addTransaction(transaction);

    if(!fromBroadcast && inserted) {
        auto broadcast = new TransactionBroadcast();
        broadcast->setSender(address);
        broadcast->setReceiver(LAddress::L3BROADCAST());
        broadcast->setLength(sizeof(Transaction));
        broadcast->setTransaction(transaction);

        application->sendInetMessage(broadcast);
    }
}

LAddress::L3Type Blockchain::getAddress() const {
    return address;
}

BlockBuilder* Blockchain::getCurrentBuilder() {
    return currentBuilder.get();
}

std::shared_ptr<Block> Blockchain::getPendingBlock() {
    if(!havePendingBlock) {
        havePendingBlock = true;
        ASSERT(currentBuilder->isFrozen());
        pendingBlock = currentBuilder->build();
    }
    return pendingBlock;
}

const NetworkView& Blockchain::makeView(const Hash& hash, int attempt) {
    if(lastView && lastView->wasGeneratedFrom(hash, attempt))
        return *lastView;
    lastView = consensusModel->generateView(hash, attempt);
    return *lastView;
}

const NetworkView& Blockchain::getAssociatedView(std::shared_ptr<Block> block) {
    return makeView(block->getPreviousHash(), block->getAttempt());
}

const NetworkView& Blockchain::getMostRecentView() {
    return makeView(getNthMostRecentBlock(0)->getHash(), attempt);
}

void Blockchain::initializeData(Blockchain* source) {
    // if the local blockchain is outdated
    EV_DEBUG << "updating blockchain";
    if(source->getNthMostRecentBlock(0)->getHeight() > getNthMostRecentBlock(0)->getHeight()) {
        // blocks are shared pointers, so a simple copy is possible and sufficient
        blocks = source->blocks;
        currentBuilder->setHeight(getNthMostRecentBlock(0)->getHeight() + 1);
        currentBuilder->setPreviousBlock(blocks.front());
        attempt = 0;
        EV_DEBUG << "now there are " << blocks.size() << " blocks";
    }
    EV_DEBUG << std::endl;
}

void Blockchain::requestBlockchainUpdate(){
    auto request = new DownloadBlockchainRequest();
    if(blocks.size() == 0)
        request->setLocalHeight(-1);
    else
        request->setLocalHeight(getNthMostRecentBlock(0)->getHeight());
    request->setLength(sizeof(int));
    // pick a random node, eventually, the blockchain will be downloaded
    request->setReceiver(addressTable->getRandomAddress());
    request->setSender(address);

    application->sendInetMessage(request);
}

const std::list<std::shared_ptr<Block>>& Blockchain::getBlocks() const {
    return blocks;
}

}
