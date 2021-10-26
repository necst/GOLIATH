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

#include "TestApplication.h"
#include "simulator/messages/ConsensusMessage_m.h"
#include "simulator/messages/NeighbourDiscovery_m.h"
#include "simulator/messages/NeighbourReply_m.h"
#include "simulator/messages/DownloadBlockchainRequest_m.h"
#include "simulator/messages/TransactionBroadcast_m.h"
#include "simulator/messages/BlockchainDownload_m.h"
#include "simulator/blockchain/Blockchain.h"
#include "simulator/blockchain/TransactionGenerator.h"
#include "simulator/blockchain/ConsensusModel.h"
#include "simulator/networking/PositionTable.h"

#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/FindModule.h"

Define_Module(framework::TestApplication);

namespace framework {

void TestApplication::initialize()
{
    blockchain = getParentModule()->getSubmodule("blockchain");
    positionTable = dynamic_cast<PositionTable*>(getModuleByPath("<root>.positionTable"));
    if(positionTable == nullptr)
        throw std::runtime_error("Position table module not found");
    // setup dynamics
    FindModule<>::findHost(this)->subscribe(BaseMobility::mobilityStateChangedSignal, this);
}

void TestApplication::handleMessage(cMessage *msg)
{
    auto blockchainStruct = dynamic_cast<Blockchain*>(blockchain->getSubmodule("blockchain"));
    auto consensusModel = dynamic_cast<ConsensusModel*>(blockchain->getSubmodule("consensusModel"));
    auto consensusMsg = dynamic_cast<ConsensusMessage*>(msg);
    if(consensusMsg != nullptr) {
        consensusModel->onMessage(
                blockchainStruct->makeView(consensusMsg->getViewSeed(), consensusMsg->getAttempt()),
                consensusMsg);
        return;
    }
    auto generator = dynamic_cast<TransactionGenerator*>(
        getParentModule()->getSubmodule("transactionGenerator"));
    auto beacon = dynamic_cast<NeighbourDiscovery*>(msg);
    if(beacon != nullptr) {
        EV_DEBUG << "Got a discovery message" << std::endl;
        generator->handleRequest(beacon);
        return;
    }

    auto reply = dynamic_cast<NeighbourReply*>(msg);
    if(reply != nullptr) {
        generator->handleReply(reply);
        return;
    }

    auto blockchainRequest = dynamic_cast<DownloadBlockchainRequest*>(msg);
    if(blockchainRequest != nullptr) {
        auto lastBlock = blockchainStruct->getNthMostRecentBlock(0);
        if(blockchainRequest->getLocalHeight() < lastBlock->getHeight()) {
            auto download = new BlockchainDownload();
            download->setSender(blockchainStruct->getAddress());
            download->setReceiver(blockchainRequest->getSender());
            download->setLength(blockchainStruct->getNthMostRecentBlock(0)->getHeight() * sizeof(Block));
            download->setBlockchainData(blockchainStruct);
            send(download, gate("inetOut"));
        }

        delete msg;
        return;
    }

    auto blockchainDownload = dynamic_cast<BlockchainDownload*>(msg);
    if(blockchainDownload != nullptr) {
        blockchainStruct->initializeData(blockchainDownload->getBlockchainData());
        delete msg;
        return;
    }

    auto transaction = dynamic_cast<TransactionBroadcast*>(msg);
    if(transaction != nullptr) {
        blockchainStruct->handleTransaction(transaction->getTransaction(), true);
        delete msg;
        return;
    }
}

void TestApplication::updatePosition(cObject* obj) {
    auto mobility = dynamic_cast<BaseMobility*>(obj);

    if(mobility == nullptr) {
        EV << "Unkown object raised mobilityStateChanged signal";
        return;
    }

    auto blockchainStruct = dynamic_cast<Blockchain*>(blockchain->getSubmodule("blockchain"));
    positionTable->updatePosition(blockchainStruct->getAddress(), mobility->getPositionAt(simTime()));

}

void TestApplication::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) {
    if(signalID == BaseMobility::mobilityStateChangedSignal) {
        updatePosition(obj);
    }
    // handle also parking signal?
}

void TestApplication::sendInetMessage(cMessage* msg) {
    Enter_Method("sendInetMessage(%s)", msg->getClassName());
    take(msg);
    send(msg, "inetOut");
}

void TestApplication::sendWaveMessage(cMessage* msg) {
    Enter_Method("sendWaveMessage(%s)", msg->getClassName());
    take(msg);
    send(msg, "waveOut");
}

}
