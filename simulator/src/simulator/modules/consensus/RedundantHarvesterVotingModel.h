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

#ifndef __SIMULATOR_REDUNDANTHARVESTERVOTINGMODEL_H_
#define __SIMULATOR_REDUNDANTHARVESTERVOTINGMODEL_H_

#include <omnetpp.h>
#include "BaseVotingModel.h"

using namespace omnetpp;

namespace framework {


class RedundantHarvesterVotingModel : public BaseVotingModel
{
protected:
    int harvesters;
    simtime_t delay;
    NetworkView currentView;
    std::vector<VotingRequest*> candidates;
    cMessage* delayMsg;

    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void processRequestPool() override;

    virtual void commit(ConsensusMessage* msg) override;

public:
    virtual std::unique_ptr<NetworkView> generateView(Hash seed, int attempt) override;
    virtual void onMessage(const NetworkView& view, cMessage* msg) override;
};
}
#endif
