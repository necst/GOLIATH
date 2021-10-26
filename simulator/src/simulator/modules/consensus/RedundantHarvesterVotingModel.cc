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

#include "RedundantHarvesterVotingModel.h"

#include <algorithm>

Define_Module(framework::RedundantHarvesterVotingModel);

namespace framework {
void RedundantHarvesterVotingModel::initialize()
{
    // use superclass initialize member function
    BaseVotingModel::initialize();
    harvesters = par("harvesters").intValue();
    delay = par("delay");
    currentView = INVALID_VIEW;
    candidates.clear();
    candidates.resize(harvesters, nullptr);
    delayMsg = new cMessage("consensus delay");
}

void RedundantHarvesterVotingModel::finish() {
    BaseVotingModel::finish();
    delete delayMsg;
    for(auto msg: candidates)
        if(msg != nullptr)
            delete msg;
}

void RedundantHarvesterVotingModel::handleMessage(cMessage *msg)
{
    if(msg == delayMsg) {
        processRequestPool();
        return;
    }
    if(msg == timeoutMsg) {
        int count = std::count(candidates.begin(), candidates.end(), nullptr);
        if(count == candidates.size()) {
            currentView = INVALID_VIEW;
            phase = Phase(std::bind(&RedundantHarvesterVotingModel::prePrepare, this, std::placeholders::_1));
            emit(registerSignal("consensusTerminated"), false);
        } else {
            // try next one
            processRequestPool();
        }
    } else
        // use superclass handleMessage member function
        BaseVotingModel::handleMessage(msg);
}

std::unique_ptr<NetworkView> RedundantHarvesterVotingModel::generateView(Hash seed, int attempt) {
    // n harvesters, 3f+1 supporters
    return std::unique_ptr<NetworkView>(new NetworkView(
            NetworkView::compute(blockchain, harvesters,  3 * faultTolerance + 1, seed, attempt)));
}

void RedundantHarvesterVotingModel::processRequestPool() {

    for(int i = 0; i < candidates.size(); ++i) {
        if(phase.getName() != Phase::PRE_PREPARE)
            break;
        if(candidates[i] != nullptr) {
            phase.react(candidates[i]);
            candidates[i] = nullptr;
        }
    }
}

void RedundantHarvesterVotingModel::commit(ConsensusMessage* msg) {
    /* pseudocode:
     * 1- register commit messages
     * 2- when committedLocal() is true, the voting has terminated and the block
     *      can be broadcasted
     */

    auto pbft = dynamic_cast<PBFTMessage*>(msg);
    if(pbft == nullptr) {
        auto request = dynamic_cast<VotingRequest*>(msg);
        if(request != nullptr)
            requestPool.push(request);
        else
            delete msg;
        return;
    }
    // accept message only if it is relevant to current phase and the node sending it
    // has accepted the block
    if(canBeProcessed(pbft) && pbft->getHasFlag() && pbft->getFlag()) {
        phase.addNode(pbft->getSender());
    }

    delete msg;
    if(committedLocal(phase)) {
        phase.addSignaturesToBlock();
        broadcast(phase.getBlock());
        blockchain->insertBlock(phase.getBlock());
        // don't reschedule the timer, as the pre-prepare phase doesn't need it
        resetTimer(false);
        phase = Phase(
            std::bind(&RedundantHarvesterVotingModel::prePrepare, this, std::placeholders::_1));
        // restore status for next round
        for(auto msg: candidates)
            delete msg;
        candidates.clear();
        candidates.resize(harvesters, nullptr);
        currentView = INVALID_VIEW;
    }
}

void RedundantHarvesterVotingModel::onMessage(const NetworkView& view, cMessage* msg) {
    auto request = dynamic_cast<VotingRequest*>(msg);
    int count = std::count(candidates.begin(), candidates.end(), nullptr);
    if(request != nullptr && count > 0 && (currentView == INVALID_VIEW || currentView == view)) {
        if(currentView == INVALID_VIEW)
            currentView = view;
        int priority = view.getHarvesterPosition(request->getRequest()->getSigner());
        candidates[priority] = request;
        if(!delayMsg->isScheduled())
            scheduleAt(simTime() + delay, delayMsg);

    } else if(request == nullptr){
        BaseVotingModel::onMessage(view, msg);
    } else
        delete msg;
}

}
