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

#ifndef __SIMULATOR_TRUSTEDENVIRONMENT_H_
#define __SIMULATOR_TRUSTEDENVIRONMENT_H_

#include <omnetpp.h>

#include "simulator/blockchain/ConsensusModel.h"

using namespace omnetpp;

namespace framework {
class TrustedEnvironment : public cSimpleModule, public ConsensusModel
{
private:
    void broadcast(std::shared_ptr<Block> block);
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
public:
    void onHarvest(const NetworkView& view, std::shared_ptr<Block> block);
    void onMessage(const NetworkView& view, omnetpp::cMessage* msg);
    bool verify(const NetworkView& view, std::shared_ptr<const Block> block);
    std::unique_ptr<NetworkView> generateView(Hash seed, int attempt);
};
}

#endif
