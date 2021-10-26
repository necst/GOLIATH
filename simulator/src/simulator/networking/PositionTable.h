/*
 * PositionTable.h
 *
 *  Created on: Jun 30, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_NETWORKING_POSITIONTABLE_H_
#define SIMULATOR_NETWORKING_POSITIONTABLE_H_

#include <map>
#include <unordered_map>

#include <omnetpp.h>

#include "veins/base/utils/SimpleAddress.h"
#include "veins/base/utils/Coord.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

#include "simulator/networking/AddressTable.h"

using namespace veins;
using namespace omnetpp;

namespace framework {

class PositionTable : public cSimpleModule {
private:
    std::unordered_map<LAddress::L3Type, Coord> positions;
    AddressTable* addressTable;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage* msg);
    virtual void finish();
public:
    static const int DELTA = 1;
    Coord getPosition(const LAddress::L3Type& id);
    void updatePosition(const LAddress::L3Type& id, Coord newPos);
    std::vector<cModule*> getNodesCloseTo(TraCICommandInterface* traci, Coord position, int range);
    void leave(const LAddress::L3Type& id);
};

} /* namespace framework */

#endif /* SIMULATOR_NETWORKING_POSITIONTABLE_H_ */
