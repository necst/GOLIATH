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

#ifndef __SIMULATOR_DOUBLETYPETRACIMANAGERLAUNCHD_H_
#define __SIMULATOR_DOUBLETYPETRACIMANAGERLAUNCHD_H_

#include <omnetpp.h>
#include "veins/modules/mobility/traci/TraCIScenarioManagerLaunchd.h"

using namespace omnetpp;
using namespace veins;

namespace framework {
class DoubleTypeTraCIManagerLaunchd : public TraCIScenarioManagerLaunchd
{
protected:
    TypeMapping secondModuleType;
    double ratio;
    std::map<std::string, int> secondTypeCounts;
    std::map<std::string, int> totalCounts;
    int cap;

    virtual void initialize(int stage) override;

    virtual void parseModuleTypes() override;
    virtual void addModule(std::string nodeId, std::string type, std::string name,
            std::string displayString, const Coord& position, std::string road_id = "",
            double speed = -1, Heading heading = Heading::nan,
            VehicleSignalSet signals = {VehicleSignal::undefined},
            double length = 0, double height = 0, double width = 0) override;
public:
    virtual bool isMainType(std::string path) const;
    virtual bool isMainType(cModule* module) const;

};
}
#endif
