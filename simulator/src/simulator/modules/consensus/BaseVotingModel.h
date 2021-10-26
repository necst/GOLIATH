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

#ifndef __SIMULATOR_BASEVOTINGMODEL_H_
#define __SIMULATOR_BASEVOTINGMODEL_H_

#include <omnetpp.h>

#include <set>
#include <functional>
#include <queue>
#include <future>

#include "simulator/blockchain/ConsensusModel.h"
#include "simulator/messages/ConsensusMessage_m.h"
#include "simulator/messages/PBFT/PBFTMessage_m.h"
#include "simulator/messages/PBFT/VotingRequest_m.h"

using namespace omnetpp;

namespace framework {

class Phase {
public:
    enum PhaseName {IDLE, PRE_PREPARE, PREPARE, COMMIT};
protected:
    PhaseName name;
    // block that is being processed
    std::shared_ptr<Block> block;
    // current view
    NetworkView view;
    // list of nodes, their purpose depends on the semantics of the phase
    std::vector<LAddress::L3Type> nodes;
    std::function<void(ConsensusMessage*)> reaction;
public:
    Phase()
        : Phase(nullptr, IDLE, INVALID_VIEW, {})
    {}
    Phase(std::function<void(ConsensusMessage*)> reaction)
        : Phase(reaction, PRE_PREPARE, INVALID_VIEW, {})
    {}
    Phase(std::function<void(ConsensusMessage*)> reaction,
            PhaseName name, const NetworkView& view,
            std::shared_ptr<Block> block)
        : name {name},
          block {block},
          view {view},
          nodes {},
          reaction {reaction}
    {}

    void addNode(LAddress::L3Type id);
    void react(ConsensusMessage* msg);
    void addSignaturesToBlock();

    std::shared_ptr<Block> getBlock() const;
    const NetworkView& getView() const;
    int countNodes() const;
    PhaseName getName() const;

};

class BaseVotingModel : public cSimpleModule, public ConsensusModel
{
protected:
    BlockValidator* validator;
    Blockchain* blockchain;
    Application* application;
    AddressTable* addressTable;
    Phase phase;
    int faultTolerance;
    simtime_t margin;
    // message pool for messages that are received before the node enters the pre-prepare phase
    std::queue<ConsensusMessage*> requestPool;
    std::vector<ConsensusMessage*> preparePool;
    std::vector<ConsensusMessage*> commitPool;
    cMessage* timeoutMsg;
    cMessage* blockTimeoutMsg;
    simtime_t timeout;
    std::shared_ptr<Block> incomingBlock;
    std::vector<LAddress::L3Type> signatures;

    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;

    void sendToSupporters(const NetworkView& view, ConsensusMessage* msg, bool virtualize);
    void sendRequestToSupporters(const NetworkView& view, std::shared_ptr<Block> block);

    bool prepared(Phase phase);
    bool committedLocal(Phase phase);
    virtual void prePrepare(ConsensusMessage* msg);
    virtual void prepare(ConsensusMessage* msg);
    virtual void commit(ConsensusMessage* msg);
    virtual void processMessagePool();
    virtual void processRequestPool();
    virtual bool canBeProcessed(PBFTMessage* msg);
    void advertiseCurrentPhase(bool hasFlag = false, bool flag = false);
    void broadcast(std::shared_ptr<Block> block);
    void broadcast(bool approved);
    void resetTimer(bool reschedule);
    bool verifySignatures(const NetworkView& view, std::vector<LAddress::L3Type> signatures);
public:
    virtual void onHarvest(const NetworkView& view, std::shared_ptr<Block> block) override;
    virtual void onMessage(const NetworkView& view, cMessage* msg) override;
    virtual bool verify(const NetworkView& view, std::shared_ptr<const Block> block) override;
    virtual std::unique_ptr<NetworkView> generateView(Hash seed, int attempt) override;
};


}
#endif
