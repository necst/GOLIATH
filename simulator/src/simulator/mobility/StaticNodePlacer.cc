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

#include "StaticNodePlacer.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "simulator/networking/AddressTable.h"

Define_Module(framework::StaticNodePlacer);

namespace framework {
void StaticNodePlacer::initialize() {
    auto placeNodes = new cMessage("placeNodes");
    auto manager = getModuleByPath("<root>.manager");
    firstStepAt = manager->par("firstStepAt");
    updateInterval = manager->par("updateInterval");
    if(firstStepAt == -1) {
        firstStepAt = manager->par("connectAt");
        firstStepAt += updateInterval;
    }
    x = par("x");
    y = par("y");
    num = par("num");

    moduleType = par("moduleType").stdstringValue();
    displayString = par("displayString").stdstringValue();
    scheduleAt(firstStepAt, placeNodes);
}

void StaticNodePlacer::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        placeNodes();
    }
    delete msg;
}

void StaticNodePlacer::placeNodes() {
    auto network = getModuleByPath("<root>");

    cModuleType* modType = cModuleType::get(moduleType.c_str());
    if(modType == nullptr)
        throw cRuntimeError("Module type \"%s\" not found", moduleType.c_str());

    for(int i = 0; i < num; ++i) {
        cModule* mod = modType->create("node2", network, i, i);
        mod->par("veinsmobilityType") = "org.car2x.veins.base.modules.BaseMobility";
        mod->finalizeParameters();
        mod->getDisplayString().parse(displayString.c_str());
        mod->buildInside();
        mod->scheduleStart(simTime() + updateInterval);

        //pre-initialize mobility parameters
        mod->getSubmodule("veinsmobility")->par("x") = (double)x;
        mod->getSubmodule("veinsmobility")->par("y") = (double)y;
        mod->getSubmodule("veinsmobility")->par("z") = 0.0;

        mod->callInitialize();

    }
}
}
