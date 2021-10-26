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

#include "SimpleRSUPlacer.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/modules/BaseMobility.h"
#include "simulator/networking/AddressTable.h"
#include "simulator/blockchain/RewardSystem.h"

#include <algorithm>

Define_Module(framework::SimpleRSUPlacer);

namespace framework {
void SimpleRSUPlacer::initialize()
{
    auto placeRSUs = new cMessage("placeRSUs");
    auto manager = getModuleByPath("<root>.manager");
    auto world = dynamic_cast<veins::BaseWorldUtility*>(getModuleByPath("<root>.world"));
    firstStepAt = manager->par("firstStepAt");
    updateInterval = manager->par("updateInterval");
    if(firstStepAt == -1) {
        firstStepAt = manager->par("connectAt");
        firstStepAt += updateInterval;
    }
    rows = par("rows");
    cols = par("cols");
    auto size = world->getPgs();
    sizeX = size->x;
    sizeY = size->y;
    rsuModuleType = par("rsuModuleType").stdstringValue();
    rsuDisplayString = par("rsuDisplayString").stdstringValue();
    scheduleAt(firstStepAt, placeRSUs);
}

void SimpleRSUPlacer::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
        placeRSUs();
    }
    delete msg;
}


void SimpleRSUPlacer::placeRSUs() {
    double stepX = sizeX / (double) (rows + 1);
    double stepY = sizeY / (double) (cols + 1);
    auto network = getModuleByPath("<root>");

    cModuleType* rsuType = cModuleType::get(rsuModuleType.c_str());
    if(rsuType == nullptr)
        throw cRuntimeError("Module type \"%s\" not found", rsuModuleType.c_str());

    for(int i = 0; i < rows; ++i) {
        double x = stepX * (i + 1);
        for(int j = 0; j < cols; ++j) {
            double y = stepY * (j + 1);
            int index = i * cols + j;
            cModule* rsu = rsuType->create("RSU", network, index, index);

            rsu->finalizeParameters();
            rsu->getDisplayString().parse(rsuDisplayString.c_str());
            rsu->buildInside();
            rsu->scheduleStart(simTime() + updateInterval);

            //pre-initialize mobility parameters
            rsu->getSubmodule("veinsmobility")->par("x") = x;
            rsu->getSubmodule("veinsmobility")->par("y") = y;
            rsu->getSubmodule("veinsmobility")->par("z") = 0.0;

            rsu->callInitialize();

            // rsus are ordered according to their index
            rsus.push_back(rsu);
        }
    }
    /* TODO: idea
     * create genesis block here and then retrieve it from Blockchain
     */
    createGenesisBlock();
    initializeRSUBlockchain();
}

void SimpleRSUPlacer::createGenesisBlock() {
    auto addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    std::vector<LAddress::L3Type> addresses;
    // register RSUs to get immediately an address
    for(auto rsu: rsus)
        addressTable->registerVehicle(rsu->getFullPath(), rsu);

    std::unique_ptr<BlockData> data(new BlockData());
    data->setHeight(0);
    data->setAttempt(0);
    double stepX = sizeX / (double) (rows + 1);
    double stepY = sizeY / (double) (cols + 1);
    auto bound = 1.5 * sqrt(stepX*stepX + stepY*stepY);
    for(auto rsu1It = rsus.begin(); rsu1It != rsus.end(); ++rsu1It) {
        auto rsu1 = *rsu1It;
        auto rsu1Address = addressTable->getAddress(rsu1->getFullPath());
        for(auto rsu2It = rsu1It + 1; rsu2It != rsus.end(); ++rsu2It) {
            auto rsu2 = *rsu2It;
            auto m1 = dynamic_cast<BaseMobility*>(rsu1->getSubmodule("veinsmobility"));
            auto m2 = dynamic_cast<BaseMobility*>(rsu2->getSubmodule("veinsmobility"));
            auto dist = m1->getPositionAt(simTime()).distance(m2->getPositionAt(simTime())) + 1;
            if(dist > bound)
                continue;
            std::shared_ptr<Transaction> t1 (new Transaction(
                    rsu1Address,
                    addressTable->getAddress(rsu2->getFullPath()),
                    m1->getPositionAt(simTime()),
                    dist, simTime()));
            data->transactions().push_back(t1);
            data->transactionInfo().push_back(TransactionInfo(true, true, "initial condition"));
            std::shared_ptr<Transaction> t2 (new Transaction(
                    addressTable->getAddress(rsu2->getFullPath()),
                    rsu1Address,
                    m2->getPositionAt(simTime()),
                    dist, simTime()));
            data->transactions().push_back(t2);
            data->transactionInfo().push_back(TransactionInfo(true, true, "initial condition"));
        }
        auto reward = dynamic_cast<RewardSystem*>(
                rsu1->getSubmodule("app")->getSubmodule("blockchain")
                ->getSubmodule("statusUpdater"));
        data->statuses().insert({
            rsu1Address,
            NodeStatus(reward->getRSUStartingReputation(), true, reward->getMaxReward())
        });
    }
    using trans_t = std::shared_ptr<const Transaction>;
    std::sort(data->transactions().begin(), data->transactions().end(),
            [] (trans_t t1, trans_t t2) { return *t1 < *t2;});
    auto size = data->transactions().size();
    genesisBlock = std::shared_ptr<Block>(new Block(size, std::move(data), size));
    genesisBlock->signBlock(addressTable->getAddress(rsus[0]->getFullPath()));
}

void SimpleRSUPlacer::initializeRSUBlockchain() {
    for(auto rsu: rsus) {
        auto blockchain = dynamic_cast<Blockchain*>(
                rsu->getSubmodule("app")->getSubmodule("blockchain")->getSubmodule("blockchain"));
        blockchain->insertBlock(genesisBlock);
    }
}

const std::vector<cModule*>& SimpleRSUPlacer::getRSUs() {
    return rsus;
}

std::shared_ptr<Block> SimpleRSUPlacer::getGenesisBlock() {
    ASSERT((bool)genesisBlock);
    return genesisBlock;
}

}
