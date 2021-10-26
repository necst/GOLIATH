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

#include "SimpleInet.h"
#include "AddressTable.h"
#include "simulator/utility/Vehicle.h"
#include "simulator/networking/BaseInetMessage_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

Define_Module(framework::SimpleInet);

namespace framework {

void SimpleInet::initialize()
{
    addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    if(addressTable == nullptr)
        throw std::runtime_error("Address table module not found");
    /*
    auto mobility = TraCIMobilityAccess().get(getParentModule());
    auto baseVehicle = mobility->getVehicleCommandInterface();
    auto vehicle = Vehicle::cast(baseVehicle);

    addressTable->registerVehicle(vehicle->getNodeId(), getParentModule()->getParentModule()->getParentModule());
    address = addressTable->getAddress(vehicle->getNodeId());*/
    auto vehicle = FindModule<>::findHost(this);
    addressTable->registerVehicle(vehicle->getFullPath(), vehicle);
    address = addressTable->getAddress(vehicle->getFullPath());
}

void SimpleInet::handleMessage(cMessage *msg)
{
    if(msg->getArrivalGate() == gate("dataIn"))
        handleMessageFromData(msg);
    else if (msg->getArrivalGate() == gate("fromApp"))
        handleMessageFromApp(msg);
    else
        delete msg;
}

void SimpleInet::handleMessageFromApp(cMessage* msg) {
    auto inetMsg = dynamic_cast<BaseInetMessage*>(msg);
    if(inetMsg == nullptr) {
        EV << "Can't send non-inet messages with inet protocol stack";
        delete msg;
        return;
    }
    inetMsg->setSender(address);

    auto transmissionTime = SimTime(inetMsg->getLength() / par("speed").doubleValue());
    transmissionTime += uniform(SimTime::ZERO, SimTime(10, SimTimeUnit::SIMTIME_MS));

    if(inetMsg->getReceiver() == LAddress::L3BROADCAST()) {
        // handle broadcast
        for(auto it : addressTable->getAllModules()) {
            if(it.second.second != nullptr && it.second.second != getParentModule())
                sendDirect(msg->dup(), 0, transmissionTime, it.second.second, "inetIn");
        }
        delete msg;
    } else {
        // handle singlecast, allow self messages
        auto receiver = addressTable->getModule(inetMsg->getReceiver());
        if(receiver != nullptr)
            sendDirect(msg, 0, transmissionTime, receiver, "inetIn");
        else {
            EV << simTime() << ": inet module can't deliver the message " << msg << " to " << inetMsg->getReceiver() << std::endl;
            delete msg;
        }
    }
}

void SimpleInet::handleMessageFromData(cMessage* msg) {
    auto inetMsg = dynamic_cast<BaseInetMessage*>(msg);
    if(inetMsg == nullptr){
        EV << "Received wrong message from dataIn";
        delete msg;
        return;
    }

    // if this node is among the receivers
    if(inetMsg->getReceiver() == address || inetMsg->getReceiver() == LAddress::L3BROADCAST())
        // forward message
        send(msg, gate("toApp"));
    else
        // else ignore it
        delete msg;
}

}
