/*
 * PositionTable.cc
 *
 *  Created on: Jun 30, 2020
 *      Author: dvdmff
 */

#include "PositionTable.h"
#include "AddressTable.h"

#include <vector>

Define_Module(framework::PositionTable);

namespace framework {

void PositionTable::initialize() {
    addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    if(addressTable == nullptr)
        throw std::runtime_error("Address table module not found");
}

void PositionTable::handleMessage(cMessage* msg) {
    // nothing to do
}

void PositionTable::finish() {
    positions.clear();
}

Coord PositionTable::getPosition(const LAddress::L3Type& id) {
    auto it = positions.find(id);
    if(it != positions.end())
        return it->second;
    return Coord::ZERO;
}

void PositionTable::updatePosition(const LAddress::L3Type& id, Coord newPos) {
    positions[id] = newPos;
}

void PositionTable::leave(const LAddress::L3Type& id) {
    positions.erase(id);
}

std::vector<cModule*> PositionTable::getNodesCloseTo(TraCICommandInterface* traci, Coord position, int range) {
    std::vector<cModule*> modules;
    for(auto it : positions) {
        auto distance = position.distance(it.second);
        if(0 < distance && distance <= range + DELTA) {
            // distance = 0 means oneself, so exclude it
            auto module = addressTable->getModule(it.first);
            // check if module is still in the network
            if(module != nullptr)
                modules.push_back(module);
        }
    }
    return modules;
}

} /* namespace framework */
