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

#include "SkipValidation.h"
#include "simulator/blockchain/RewardSystem.h"

#include "veins/base/utils/FindModule.h"

Define_Module(framework::SkipValidation);

namespace framework {
void SkipValidation::initialize()
{
    blockchain = dynamic_cast<Blockchain*>(getParentModule()->getSubmodule("blockchain"));
}

void SkipValidation::handleMessage(cMessage *msg)
{
    // do nothing
}


void SkipValidation::validate(BlockBuilder& builder) {

    builder.stopAddingTransactions();
    builder.prepare();

    auto reward = dynamic_cast<RewardSystem*>(getParentModule()->getSubmodule("statusUpdater"));

    for(size_t i = 0; i < builder.data().transactions().size(); ++i) {
        builder.setTransactionInfo(i, TransactionInfo(true, true, "skip validation"));
        // add new nodes if not present already
        auto& statuses = builder.data().statuses();
        auto sender = builder.data().transactions()[i]->getSender();
        auto target = builder.data().transactions()[i]->getTarget();
        if(statuses.find(sender) == statuses.end())
            builder.updateStatus(sender,
                    NodeStatus(reward->getStartingReputation(), true, reward->getMaxReward()));
        if(statuses.find(target) == statuses.end())
            builder.updateStatus(target,
                    NodeStatus(reward->getStartingReputation(), true, reward->getMaxReward()));
    }
    //no reputation changes

    // block is now ready for the consensus process
    emit(registerSignal("blockValidated"), true);
}

bool SkipValidation::validate(std::shared_ptr<const Block> block) {
    // trust the environment
    return true;
}


}
