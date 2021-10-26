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

#include "TrustedEnvironment.h"

#include "simulator/blockchain/Blockchain.h"
#include "simulator/application/Application.h"
#include "simulator/messages/NewBlock_m.h"

#include "veins/base/utils/FindModule.h"

#include <stdexcept>

Define_Module(framework::TrustedEnvironment);


namespace framework {
void TrustedEnvironment::initialize()
{
    // do nothing
}

void TrustedEnvironment::handleMessage(cMessage *msg)
{
    // do nothing
}

void TrustedEnvironment::broadcast(std::shared_ptr<Block> block) {
    NewBlock* msg = new NewBlock();
    msg->setBlock(block);
    msg->setReceiver(LAddress::L3BROADCAST());
    msg->setLength(block->size());
    msg->setViewSeed(block->getHash());
    msg->setAttempt(block->getAttempt());
    // send through application socket
    auto app = FindModule<>::findHost(this);
    app = app->getSubmodule("app")->getSubmodule("application");
    auto application = dynamic_cast<Application*>(app);
    application->sendInetMessage(msg);
}

void TrustedEnvironment::onHarvest(const NetworkView& view, std::shared_ptr<Block> block) {
    // seal block and broadcast it

    Blockchain* blockchain = dynamic_cast<Blockchain*>(getParentModule()->getSubmodule("blockchain"));
    ASSERT(blockchain != nullptr);

    auto address = blockchain->getAddress();

    if(view.isHarvester(address)) {
        block->signBlock(address);
        broadcast(block);
        emit(registerSignal("consensusTerminated"), true);
        blockchain->insertBlock(block);
    }
}

void TrustedEnvironment::onMessage(const NetworkView& view, omnetpp::cMessage* msg) {
    auto newBlock = dynamic_cast<NewBlock*>(msg);
    auto blockchain = dynamic_cast<Blockchain*>(getParentModule()->getSubmodule("blockchain"));
    auto block = newBlock->getBlock();

    if(newBlock != nullptr){
        // no protocol to run, just add new block when it arrives
        emit(registerSignal("consensusTerminated"), true);
        blockchain->insertBlock(block);
    }
    delete msg;
}

bool TrustedEnvironment::verify(const NetworkView& view, std::shared_ptr<const Block> block) {
    // just check if the block is signed by the correct node
    return view.isHarvester(block->getSigner());
}

std::unique_ptr<NetworkView> TrustedEnvironment::generateView(Hash seed, int attempt) {
    auto blockchain = dynamic_cast<Blockchain*>(getParentModule()->getSubmodule("blockchain"));
    return std::unique_ptr<NetworkView>(new NetworkView(NetworkView::compute(blockchain, 1, 0, seed, attempt)));
}

}
