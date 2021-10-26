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

#include <vector>

#include "SimpleWave.h"
#include "AddressTable.h"
#include "PositionTable.h"
#include "simulator/utility/Vehicle.h"
#include "simulator/networking/BaseWaveMessage_m.h"

#include "veins/base/utils/FindModule.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

Define_Module(framework::SimpleWave);


namespace framework {
void SimpleWave::initialize()
{
    addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    if(addressTable == nullptr)
        throw std::runtime_error("Address table module not found");
    positionTable = dynamic_cast<PositionTable*>(getModuleByPath("<root>.positionTable"));
    if(positionTable == nullptr)
        throw std::runtime_error("Position table module not found");
    /*auto mobility = TraCIMobilityAccess().get(getParentModule());
    auto baseVehicle = mobility->getVehicleCommandInterface();
    auto vehicle = Vehicle::cast(baseVehicle);
    traci = mobility->getCommandInterface();

    addressTable->registerVehicle(vehicle->getNodeId(), FindModule<>::findHost(this));
    address = addressTable->getAddress(vehicle->getNodeId());*/

    auto vehicle = FindModule<>::findHost(this);
    addressTable->registerVehicle(vehicle->getFullPath(), vehicle);
    address = addressTable->getAddress(vehicle->getFullPath());
}

void SimpleWave::handleMessage(cMessage *msg)
{
    if(msg->getArrivalGate() == gate("radioIn"))
        handleMessageFromRadio(msg);
    else if (msg->getArrivalGate() == gate("fromApp"))
        handleMessageFromApp(msg);
    else
        delete msg;
}

void SimpleWave::handleMessageFromRadio(cMessage* msg) {
    auto waveMsg = dynamic_cast<BaseWaveMessage*>(msg);
    if(waveMsg == nullptr) {
        EV << "Received wrong message from radioIn";
        delete msg;
        return;
    }

    // if this node is among the receivers
    if(waveMsg->getReceiver() == address || waveMsg->getReceiver() == LAddress::L3BROADCAST())
        // forward message
        send(msg, gate("toApp"));
    else
        // else ignore it
        delete msg;
}

void SimpleWave::handleMessageFromApp(cMessage* msg) {
    auto waveMsg = dynamic_cast<BaseWaveMessage*>(msg);
    if(waveMsg == nullptr) {
        EV << "Can't send non-wave messages with wave protocol stack";
        delete msg;
        return;
    }
    waveMsg->setSender(address);

    auto transmissionTime = SimTime(waveMsg->getLength() / par("speed").doubleValue());
    transmissionTime += uniform(SimTime::ZERO, SimTime(10, SimTimeUnit::SIMTIME_MS));

    if(waveMsg->getReceiver() == LAddress::L3BROADCAST()) {
        // optimized broadcast to avoid sending too many messages
        auto modules = positionTable->getNodesCloseTo(traci,
                positionTable->getPosition(address), par("range").intValue());
        for(size_t i = 0; i < modules.size(); ++i)
            if(modules[i] != getParentModule()) {
                if(i == modules.size() - 1)
                    sendDirect(msg, 0, transmissionTime, modules[i], "dsrcIn");
                else
                    sendDirect(msg->dup(), 0, transmissionTime, modules[i], "dsrcIn");
            }
        if(modules.size() == 0)
            // no one to send message to, drop it
            delete msg;
    } else {
        // just send to receiver if in range, allow self messages
        auto receiver = addressTable->getModule(waveMsg->getReceiver());
        auto receiverPos = positionTable->getPosition(waveMsg->getReceiver());
        auto senderPos = positionTable->getPosition(address);
        if(senderPos.distance(receiverPos) <= par("range").intValue() + PositionTable::DELTA)
            sendDirect(msg, 0, transmissionTime, receiver, "dsrcIn");
        else
            delete msg;
    }

}

}

