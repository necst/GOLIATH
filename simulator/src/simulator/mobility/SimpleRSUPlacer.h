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

#ifndef __SIMULATOR_SIMPLERSUPLACER_H_
#define __SIMULATOR_SIMPLERSUPLACER_H_

#include <omnetpp.h>

#include "RSUPlacer.h"
#include "simulator/blockchain/Block.h"

using namespace omnetpp;

namespace framework {
class SimpleRSUPlacer : public cSimpleModule, public RSUPlacer
{
protected:
    simtime_t firstStepAt;
    simtime_t updateInterval;
    int rows;
    int cols;
    double sizeX;
    double sizeY;
    std::string rsuModuleType;
    std::string rsuDisplayString;
    std::vector<cModule*> rsus;
    std::shared_ptr<Block> genesisBlock;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    void createGenesisBlock();
    void initializeRSUBlockchain();
    void placeRSUs();
public:
    virtual const std::vector<cModule*>& getRSUs() override;
    virtual std::shared_ptr<Block> getGenesisBlock() override;
};
}

#endif
