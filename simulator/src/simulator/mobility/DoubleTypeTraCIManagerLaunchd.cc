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

#include "DoubleTypeTraCIManagerLaunchd.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

#include <string>
#include <sstream>
#include <limits>

Define_Module(framework::DoubleTypeTraCIManagerLaunchd);

namespace framework {

void DoubleTypeTraCIManagerLaunchd::initialize(int stage) {
    if(stage != 1)
        TraCIScenarioManagerLaunchd::initialize(stage);

    ratio = par("ratio");
    cap = par("vehicleCap");
    if(cap < 0)
        cap = std::numeric_limits<int>::max();

    TraCIScenarioManagerLaunchd::initialize(stage);
}

void DoubleTypeTraCIManagerLaunchd::parseModuleTypes() {
    TraCIScenarioManagerLaunchd::parseModuleTypes();

    TypeMapping::iterator i;
    std::string secondModuleTypes = par("secondModuleType").stdstringValue();
    secondModuleType = parseMappings(secondModuleTypes, "secondModuleType", false);

    std::vector<std::string> secondTypeKeys;
    std::vector<std::string> typeKeys;

    for (i = secondModuleType.begin(); i != secondModuleType.end(); i++) secondTypeKeys.push_back(i->first);
    for (i = moduleType.begin(); i != moduleType.end(); i++) typeKeys.push_back(i->first);

    std::vector<std::string> intersection;
    std::set_intersection(typeKeys.begin(),
            typeKeys.end(), secondTypeKeys.begin(),
            secondTypeKeys.end(), std::back_inserter(intersection));
    if (intersection.size() != typeKeys.size() || intersection.size() != secondTypeKeys.size())
        throw cRuntimeError("keys of mappings of moduleType and secondModuleType are not the same");

}
void DoubleTypeTraCIManagerLaunchd::addModule(std::string nodeId, std::string type, std::string name,
            std::string displayString, const Coord& position, std::string road_id,
            double speed, Heading heading, VehicleSignalSet signals, double length,
            double height, double width) {

    auto secondTypeCountIt = secondTypeCounts.insert({type, 0});
    auto countIt = totalCounts.insert({type, 0});

    if(countIt.first != totalCounts.end() && countIt.first->second >= cap)
        return;

    countIt.first->second += 1;

    if(totalCounts[type] > 0
            && (secondTypeCountIt.first->second + 1) / (double)(countIt.first->second + 1) <= ratio) {
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

bool DoubleTypeTraCIManagerLaunchd::isMainType(std::string path) const {
    std::stringstream ss{path};
    std::string token;
    while(std::getline(ss, token, '.')) {
        if(token.size() == 0)
            continue;
        for(auto nameIt: moduleName) {
            auto mod = nameIt.second;
            if(token.find(nameIt.second + "2") == 0)
                return false;
            else if(token.find(nameIt.second) == 0)
                return true;
        }
    }
    // default to true
    return true;
}

bool DoubleTypeTraCIManagerLaunchd::isMainType(cModule* module) const {
    return isMainType(module->getFullPath());
}

}
