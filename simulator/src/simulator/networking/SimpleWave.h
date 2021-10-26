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

#ifndef __SIMULATOR_SIMPLEWAVE_H_
#define __SIMULATOR_SIMPLEWAVE_H_

#include <omnetpp.h>

#include "veins/base/utils/SimpleAddress.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "simulator/networking/AddressTable.h"
#include "simulator/networking/PositionTable.h"

using namespace omnetpp;
using namespace veins;

namespace framework {
class SimpleWave : public cSimpleModule
{
private:
    LAddress::L3Type address;
    TraCICommandInterface* traci;
    AddressTable* addressTable;
    PositionTable* positionTable;

    void handleMessageFromApp(cMessage* msg);
    void handleMessageFromRadio(cMessage* msg);
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};
}

#endif
