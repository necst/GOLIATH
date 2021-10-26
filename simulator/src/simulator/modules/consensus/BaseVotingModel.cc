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

#include "BaseVotingModel.h"
#include "simulator/messages/PBFT/VotingRequest_m.h"
#include "simulator/messages/PBFT/PBFTApprovalSignature_m.h"
#include "simulator/messages/NewBlock_m.h"
#include "simulator/utility/functions.h"

#include "veins/base/utils/FindModule.h"

#include <algorithm>
#include <map>

Define_Module(framework::BaseVotingModel);

namespace framework {

void Phase::addNode(LAddress::L3Type id) {
    int viewCount = std::count_if(
            view.supportersBegin(),
            view.supportersEnd(),
            [id] (LAddress::L3Type a) { return a == id; });
    int nodeCount = std::count_if(
            nodes.begin(),
            nodes.end(),
            [id] (LAddress::L3Type a) { return a == id; });
    // keep node list consistent with view
    if(nodeCount < viewCount)
        nodes.push_back(id);
}

void Phase::react(ConsensusMessage* msg) {
    reaction(msg);
}

void Phase::addSignaturesToBlock() {
    for(auto n: nodes)
        block->addSupporter(n);
}

std::shared_ptr<Block> Phase::getBlock() const {
    return block;
}

const NetworkView& Phase::getView() const {
    return view;
}

int Phase::countNodes() const {
    return nodes.size();
}

Phase::PhaseName Phase::getName() const {
    return name;
}


void BaseVotingModel::initialize()
{
    phase = Phase(std::bind(&BaseVotingModel::prePrepare, this, std::placeholders::_1));
    blockchain = dynamic_cast<Blockchain*>(getModuleByPath("^.blockchain"));
    ASSERT(blockchain != nullptr);
    validator = dynamic_cast<BlockValidator*>(getModuleByPath("^.blockValidator"));
    ASSERT(validator != nullptr);
    auto app = FindModule<>::findHost(this)->getModuleByPath(".app.application");
    application = dynamic_cast<Application*>(app);
    ASSERT(application != nullptr);
    addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    ASSERT(addressTable != nullptr);
    faultTolerance = par("faultTolerance").intValue();
    timeoutMsg = new cMessage("consensus timeout");
    blockTimeoutMsg = new cMessage();
    timeout = SimTime(par("timeout").intValue(), SimTimeUnit::SIMTIME_S);
    margin = SimTime(par("margin").intValue(), SimTimeUnit::SIMTIME_MS);
}

void BaseVotingModel::finish() {
    cancelAndDelete(timeoutMsg);
    cancelAndDelete(blockTimeoutMsg);
    for(auto msg: preparePool)
        delete msg;
    for(auto msg: commitPool)
        delete msg;
    while(requestPool.size() > 0) {
        delete requestPool.front();
        requestPool.pop();
    }
}

void BaseVotingModel::handleMessage(cMessage *msg)
{
    if(msg == timeoutMsg) {
        EV_DEBUG << "Consensus process failed" << std::endl;
        //ASSERT(false);
        // timeout elapsed, consensus failed, so reset status
        phase = Phase(std::bind(&BaseVotingModel::prePrepare, this, std::placeholders::_1));
        emit(registerSignal("consensusTerminated"), false);
        // don't delete timeoutMsg, as it will be reused for next consensus round
        // this round has failed, try processing another request
    } else if(msg == blockTimeoutMsg) {
        //ASSERT(false);
        signatures.clear();
        incomingBlock.reset();
        emit(registerSignal("consensusTerminated"), false);
    } else {
        EV_DEBUG << "unknown message received" << msg << std::endl;
        delete msg;
    }
}

void BaseVotingModel::sendRequestToSupporters(const NetworkView& view, std::shared_ptr<Block> block) {
    ASSERT(block->hasData());
    VotingRequest* request = new VotingRequest();
    request->setRequest(block);
    request->setLength(block->size());
    request->setViewSeed(block->getPreviousHash());
    request->setAttempt(block->getAttempt());
    sendToSupporters(view, request, false);
}

void BaseVotingModel::sendToSupporters(const NetworkView& view, ConsensusMessage* msg, bool virtualize) {
    // count how many times this node appears among the supporters, so that this node
    // can simulate n other nodes
    ASSERT(view != INVALID_VIEW);
    int count = 1;
    if(virtualize)
        count = std::count(
            view.supportersBegin(),
            view.supportersEnd(),
            blockchain->getAddress());
            //[this] (LAddress::L3Type a) { return a == blockchain->getAddress(); });

    std::set<LAddress::L3Type> uniqueSupporters(view.supportersBegin(), view.supportersEnd());
    for(auto s: uniqueSupporters/*auto it = view.supportersBegin(); it != view.supportersEnd(); ++it*/) {
        msg->setReceiver(s/**it*/);
        for(int i = 0; i < count; ++i) {
            /*if(it + 1 == view.supportersEnd() && i + 1 == count)
                application->sendInetMessage(msg);
            else*/
                application->sendInetMessage(msg->dup());
            Enter_Method_Silent();
        }
    }
    delete msg;
}

bool BaseVotingModel::prepared(Phase phase) {
    return phase.getName() == Phase::PREPARE && phase.countNodes() > 2 * faultTolerance;
}

bool BaseVotingModel::committedLocal(Phase phase) {
    return phase.getName() == Phase::COMMIT && phase.countNodes() > 2 * faultTolerance;
}

void BaseVotingModel::processMessagePool() {
    if(phase.getName() == Phase::PREPARE) {
        // filter out unusable messages and run program on remaining
        // messages until they are all processed or phase changed
        for(auto it = preparePool.begin();
                phase.getName() == Phase::PREPARE && it != preparePool.end();
                ++it)
            phase.react(*it);
        // any other message isn't necessary
        preparePool.clear();

    } else if(phase.getName() == Phase::COMMIT) {
        // filter out unusable messages and run program on remaining
        // messages until they are all processed or phase changed

        for(auto it = commitPool.begin();
                phase.getName() == Phase::COMMIT && it != commitPool.end();
                ++it)
            phase.react(*it);
        // any other message isn't necessary
        commitPool.clear();
    }
}

void BaseVotingModel::processRequestPool() {
    // process all requests in FIFO order until there are no more left or
    // the node exits the pre-prepare phase
    while(phase.getName() == Phase::PRE_PREPARE && requestPool.size() > 0) {
        auto request = requestPool.front();
        requestPool.pop();
        phase.react(request);
    }
}

void BaseVotingModel::advertiseCurrentPhase(bool hasFlag, bool flag) {
    auto msg = new PBFTMessage(("phase" + std::to_string(phase.getName())).c_str());
    msg->setPhase(phase.getName());
    msg->setBlockHash(phase.getBlock()->getHash());
    msg->setViewSeed(phase.getBlock()->getPreviousHash());
    msg->setAttempt(phase.getBlock()->getAttempt());
    msg->setLength(sizeof(int) * 2
            + sizeof(char) * phase.getBlock()->getHash().size() * 2);
    msg->setHasFlag(hasFlag);
    msg->setFlag(flag);
    sendToSupporters(phase.getView(), msg, true);
}

void BaseVotingModel::broadcast(std::shared_ptr<Block> block) {
    auto newBlock = new NewBlock();
    newBlock->setBlock(block);
    newBlock->setReceiver(LAddress::L3BROADCAST());
    newBlock->setLength(block->size());
    newBlock->setViewSeed(block->getPreviousHash());
    newBlock->setAttempt(block->getAttempt());
    auto selfMsg = newBlock->dup();
    selfMsg->setReceiver(blockchain->getAddress());
    application->sendInetMessage(newBlock);
    application->sendInetMessage(selfMsg);
    Enter_Method_Silent();
}

void BaseVotingModel::broadcast(bool approved) {
    auto signature = new PBFTApprovalSignature();
    signature->setValid(approved);
    signature->setReceiver(LAddress::L3BROADCAST());
    signature->setLength(sizeof(long long));
    signature->setViewSeed(phase.getBlock()->getPreviousHash());
    signature->setAttempt(phase.getBlock()->getAttempt());
    auto selfMsg = signature->dup();
    selfMsg->setReceiver(blockchain->getAddress());
    application->sendInetMessage(signature);
    application->sendInetMessage(selfMsg);
    Enter_Method_Silent();
}

void BaseVotingModel::resetTimer(bool reschedule) {
    Enter_Method_Silent();
    cancelEvent(timeoutMsg);
    if(reschedule)
        scheduleAt(simTime() + timeout, timeoutMsg);
}

void BaseVotingModel::prePrepare(ConsensusMessage* msg) {
    /* pseudocode:
     * 1- check that
     *   a- harvester is correct
     *   b- view is correct
     *   c- there's no other block for this view (can cache the extra blocks for later)
     *   d- attempt is coherent with time
     * 2- if all above is ok, send prepare to supporters and change phase
     *
     * side- register prepare and commit messages that are plausible to receive
     */

    //auto request = dynamic_cast<VotingRequest*>(msg);
    auto request = dynamic_cast<NewBlock*>(msg);

    if((bool)phase.getBlock()) {
        requestPool.push(request);
        EV_DEBUG << "can't start (already doing)" << std::endl;
        return;
    }

    if(request == nullptr) {
        auto pbftMsg = dynamic_cast<PBFTMessage*>(msg);
        if(pbftMsg != nullptr && pbftMsg->getPhase() == Phase::PREPARE) {
            preparePool.push_back(pbftMsg);
        } else if (pbftMsg != nullptr && pbftMsg->getPhase() == Phase::COMMIT) {
            commitPool.push_back(pbftMsg);
        } else
            delete msg;
        return;
    }

    auto& view = blockchain->makeView(request->getViewSeed(), request->getAttempt());
    if(!view.isHarvester(request->getSender())) {
        delete msg;
        EV_DEBUG << "can't start (harvester)" << std::endl;
        return;
    }
    std::shared_ptr<Block> block = request->getBlock();
    // don't start a vote on a block that is already too old
    if(block->getHeight() <= blockchain->getNthMostRecentBlock(0)->getHeight()){
        delete msg;
        EV_DEBUG << "can't start (height)" << std::endl;
        return;
    }

    // check if the view is correct and the block is the next to be processed
    // obs: this allows multiple harvesters to send their blocks
    if(view != blockchain->getAssociatedView(block)
            || !hashEqual(blockchain->getNthMostRecentBlock(0)->getHash(),
                    block->getPreviousHash())) {
        delete msg;
        EV_DEBUG << "can't start (view)" << std::endl;
        return;
    }

    ASSERT(view.isSupporter(blockchain->getAddress()));

    simtime_t elapsedTime =
            block->getCreationTime() - blockchain->getNthMostRecentBlock(0)->getCreationTime();
    elapsedTime /= (block->getAttempt() + 1);
    simtime_t blocktime = SimTime(getParentModule()->par("blocktime").intValue(), SimTimeUnit::SIMTIME_S);
    simtime_t epsilon = std::max(elapsedTime, blocktime) - std::min(elapsedTime, blocktime);
    if(epsilon > margin) {
        delete msg;
        EV_DEBUG << "can't start (elapsed time)" << std::endl;
        return;
    }

    delete msg;
    if(!incomingBlock) {
        incomingBlock = block;
        //signatures.clear();
    }
    resetTimer(true);
    phase = Phase(
            std::bind(&BaseVotingModel::prepare, this, std::placeholders::_1),
            Phase::PREPARE, view, block);
    //phase.addNode(blockchain->getAddress());
    advertiseCurrentPhase();
    processMessagePool();
}

bool BaseVotingModel::canBeProcessed(PBFTMessage* msg) {
    return msg->getPhase() == phase.getName()
            && hashEqual(msg->getViewSeed(), phase.getBlock()->getPreviousHash())
            && hashEqual(msg->getBlockHash(), phase.getBlock()->getHash())
            && msg->getAttempt() == phase.getBlock()->getAttempt();
}

void BaseVotingModel::prepare(ConsensusMessage* msg) {
    /* pseudocode:
     * 1- register prepare messages
     * 2- when prepared() is true, send commit to supporters and change phase
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

    if(canBeProcessed(pbft)) {
        phase.addNode(pbft->getSender());
    } else if(pbft->getPhase() == Phase::COMMIT) {
        commitPool.push_back(pbft);
        return;
    }

    delete msg;
    if(prepared(phase)) {
        /*bool valid = validator->validate(phase.getBlock());
        if(!valid)
            EV_DEBUG << "reject" << std::endl;*/
        resetTimer(true);
        phase = Phase(
                std::bind(&BaseVotingModel::commit, this, std::placeholders::_1),
                Phase::COMMIT,
                phase.getView(),
                phase.getBlock());
        //phase.addNode(blockchain->getAddress());
        advertiseCurrentPhase();
        processMessagePool();
    }
}

void BaseVotingModel::commit(ConsensusMessage* msg) {
    /* pseudocode:
     * 1- register commit messages
     * 2- when committedLocal() is true, the voting has terminated and the block
     *      approval signature can be broadcasted
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
    if(canBeProcessed(pbft)) {
        phase.addNode(pbft->getSender());
    }

    delete msg;
    if(committedLocal(phase)) {
        // can now send the validation outcome
        bool valid = validator->validate(phase.getBlock());
        auto count = std::count(
            phase.getView().supportersBegin(),
            phase.getView().supportersEnd(),
            blockchain->getAddress());
        for(auto i = 0; i < count; ++i)
            broadcast(valid);
        // don't reschedule the timer, as the pre-prepare phase doesn't need it
        resetTimer(false);
        if(valid) {
            // add this node's signatures to signature list
            /*for(auto i = 0; i < count; ++i)
                signatures.push_back(blockchain->getAddress());*/
        } else
            EV_DEBUG << "rejected block " << phase.getBlock()->getHeight()
                << " attempt " << phase.getBlock()->getAttempt() <<  std::endl;
        phase = Phase(
            std::bind(&BaseVotingModel::prePrepare, this, std::placeholders::_1));
    }
}

void BaseVotingModel::onHarvest(const NetworkView& view, std::shared_ptr<Block> block) {
    if(view.isHarvester(blockchain->getAddress())) {
        block->signBlock(blockchain->getAddress());
        //sendRequestToSupporters(view, block);
        broadcast(block);
        // setup harvester
        /*incomingBlock = block;
        signatures.clear();
        // give enough time for the 3 phases
        scheduleAt(simTime() + timeout * 3, blockTimeoutMsg);*/
    }
}

bool BaseVotingModel::verifySignatures(const NetworkView& view,
        std::vector<LAddress::L3Type> signatures) {
    // the count of the signatures form each supporter must match with the number of times
    // they appear in the view
    ASSERT(signatures.size() < view.supportersEnd() - view.supportersBegin());
    if(view == INVALID_VIEW || signatures.size() <= faultTolerance)
        // it's impossible to reach quorum
        return false;
    std::map<LAddress::L3Type, int> signatureCounts;
    for(auto it: signatures) {
        auto outcome = signatureCounts.insert({it, 1});
        if(!outcome.second)
            outcome.first->second += 1;
    }
    int matches = 0;
    for(auto supporter = view.supportersBegin(); supporter != view.supportersEnd(); ++supporter) {
        auto it = signatureCounts.find(*supporter);
        if(it != signatureCounts.end() && it->second > 0) {
            ++matches;
            it->second -= 1;
        }
    }
    return matches > faultTolerance;
}

bool BaseVotingModel::verify(const NetworkView& view, std::shared_ptr<const Block> block) {
    return verifySignatures(view, block->getSupporters());
}

std::unique_ptr<NetworkView> BaseVotingModel::generateView(Hash seed, int attempt) {
    // single harvester, 3f+1 supporters
    return std::unique_ptr<NetworkView>(new NetworkView(
            NetworkView::compute(blockchain, 1,  3 * faultTolerance + 1, seed, attempt)));

}

void BaseVotingModel::onMessage(const NetworkView& view, cMessage* msg) {
    Enter_Method_Silent();

    if(view == INVALID_VIEW) {
        delete msg;
        return;
    }

    auto newBlock = dynamic_cast<NewBlock*>(msg);
    if(newBlock != nullptr && !view.isSupporter(blockchain->getAddress())
            && view.isHarvester(newBlock->getBlock()->getSigner())) {

	incomingBlock = newBlock->getBlock();
        //ASSERT(incomingBlock->getAttempt() == 0);
        signatures.clear();
        // give enough time for the 3 phases (2 timeouts + 1)
        if(blockTimeoutMsg->isScheduled())
            cancelEvent(blockTimeoutMsg);
        scheduleAt(simTime() + timeout * 3, blockTimeoutMsg);
        delete msg;
        return;
    }

    auto signature = dynamic_cast<PBFTApprovalSignature*>(msg);
    if(signature != nullptr && incomingBlock) {
        auto blockView = blockchain->getAssociatedView(incomingBlock);
        if(view == blockView && signature->getValid() && view.isSupporter(signature->getSender())) {
            signatures.push_back(signature->getSender());
            if(verifySignatures(view, signatures)) {
                if(blockTimeoutMsg->isScheduled())
                    cancelEvent(blockTimeoutMsg);
                if(!verify(view, incomingBlock))
                    for(auto s: signatures)
                        if(view.isSupporter(s))
                            incomingBlock->addSupporter(s);

                signatures.clear();
                emit(registerSignal("consensusTerminated"), true);
                blockchain->insertBlock(incomingBlock);
                incomingBlock.reset();
            }
        }
        delete msg;
        return;
    }

    auto consensusMsg = dynamic_cast<ConsensusMessage*>(msg);
    if(consensusMsg != nullptr && view.isSupporter(blockchain->getAddress())) {
        phase.react(consensusMsg);
        return;
    }

    delete msg;
}

}
