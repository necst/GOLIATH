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

#include "MaliciousGroupTraCIManagerLaunchd.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

Define_Module(framework::MaliciousGroupTraCIManagerLaunchd);

namespace framework {
void MaliciousGroupTraCIManagerLaunchd::initialize(int stage) {
    if(stage != 1)
        DoubleTypeTraCIManagerLaunchd::initialize(stage);

    onRoad = par("onRoad");
    DoubleTypeTraCIManagerLaunchd::initialize(stage);
}

void MaliciousGroupTraCIManagerLaunchd::addModule(std::string nodeId, std::string type, std::string name,
        std::string displayString, const Coord& position, std::string road_id,
        double speed, Heading heading,
        VehicleSignalSet signals,
        double length, double height, double width) {

    auto secondTypeCountIt = secondTypeCounts.insert({type, 0});
    auto countIt = totalCounts.insert({type, 0});

    if(countIt.first != totalCounts.end() && countIt.first->second >= cap)
        return;

    countIt.first->second += 1;

    if(totalCounts[type] > 0 && (onRoad < 0 || countIt.first->second < onRoad)) {
        std::string vType = commandIfc->vehicle(nodeId).getTypeId();
        auto it = secondModuleType.find(vType);
        if(it == secondModuleType.end()) {
            it = secondModuleType.find("*");
            vType = "*";
        }
        name = name + "2";
        type = it->second;
        secondTypeCountIt.first->second += 1;
    }

    TraCIScenarioManagerLaunchd::addModule(nodeId, type, name, displayString,
        position, road_id, speed, heading,
        signals, length, height, width);

}

}
