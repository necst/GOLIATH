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

#include "OptimizedBlockchain.h"

#include "simulator/mobility/DoubleTypeTraCIManagerLaunchd.h"

Define_Module(framework::OptimizedBlockchain);

namespace framework {

void OptimizedBlockchain::initialize(int stage) {
    if(stage != 1){
        Blockchain::initialize(stage);
        return;
    }
    allocator = dynamic_cast<BuilderAllocator*>(getModuleByPath("<root>.allocator"));
    ASSERT(allocator != nullptr);
    filter = dynamic_cast<TransactionFilter*>(getModuleByPath("^.filter"));
    ASSERT(allocator != nullptr);
    auto manager = dynamic_cast<DoubleTypeTraCIManagerLaunchd*>(getModuleByPath("<root>.manager"));
    if(par("allowAlternativeBuilder").boolValue() && manager != nullptr)
        useMainBuilder = manager->isMainType(FindModule<>::findHost(this));
    else
        useMainBuilder = true;
    updateBuilders(1);
    Blockchain::initialize(stage);
}

void OptimizedBlockchain::handleTransaction(std::shared_ptr<const Transaction> transaction,
        bool fromBroadcast) {

    if(!fromBroadcast) {
        if(currentBuilder->isFrozen())
            nextBuilder->addTransaction(transaction);
        else
            currentBuilder->addTransaction(transaction);
    }
}

void OptimizedBlockchain::finish() {
    Blockchain::finish();
    if(currentBuilder)
        currentBuilder.reset();
    if(nextBuilder)
        nextBuilder.reset();
}

void OptimizedBlockchain::initializeData(Blockchain* source) {
    // if the local blockchain is outdated
    EV_DEBUG << "updating blockchain";
    if(source->getNthMostRecentBlock(0)->getHeight() > getNthMostRecentBlock(0)->getHeight()) {
        // blocks are shared pointers, so a simple copy is possible and sufficient
        blocks = source->getBlocks();
        auto tmp = currentBuilder;
        updateBuilders(getNthMostRecentBlock(0)->getHeight() + 1);
        currentBuilder->take(*tmp);
        currentBuilder->setPreviousBlock(blocks.front());
        attempt = 0;
        EV_DEBUG << "now there are " << blocks.size() << " blocks";
    }
    EV_DEBUG << std::endl;
}

void OptimizedBlockchain::receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) {
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
            /*if(getCurrentBuilder()->data().transactions().size() < blocksize)
                currentBuilder->fillUpWith(getNthMostRecentBlock(0));*/

            // if this node is the harvester, start the validation procedure
            validator->validate(*getCurrentBuilder());
        } else
            // just update the view in case the block gets rejected
            attempt += 1;
    }
}

BlockBuilder* OptimizedBlockchain::getCurrentBuilder() {
    return currentBuilder.get();
}

std::shared_ptr<Block> OptimizedBlockchain::getPendingBlock() {
    if(!havePendingBlock) {
        havePendingBlock = true;
        ASSERT(currentBuilder->isFrozen());
        pendingBlock = currentBuilder->build();
    }
    return pendingBlock;
}

void OptimizedBlockchain::prepareNextRound() {
    attempt = 0;
    // update builders
    updateBuilders(getNthMostRecentBlock(0)->getHeight() + 1);
    currentBuilder->setAttempt(0);
    currentBuilder->setPreviousBlock(getNthMostRecentBlock(0));
    havePendingBlock = false;
    // drop pointer to allow block to be freed through RAII
    pendingBlock.reset();
    harvester->reset();
    // reset may require a context change
    Enter_Method_Silent();
}

void OptimizedBlockchain::updateBuilders(int height) {
    if(useMainBuilder) {
        currentBuilder = allocator->getBuilderForHeight(
                height, blocksize, filter);
        nextBuilder = allocator->getBuilderForHeight(
                height + 1, blocksize, filter);
    } else {
        currentBuilder = allocator->getAlternativeBuilderForHeight(
                height, blocksize, filter);
        nextBuilder = allocator->getAlternativeBuilderForHeight(
                height + 1, blocksize, filter);
    }
    currentBuilder->setHeight(height);
    nextBuilder->setHeight(height + 1);
}

}
