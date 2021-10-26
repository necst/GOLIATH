/*
 * Vehcile.h
 *
 *  Created on: Jun 30, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_UTILITY_VEHICLE_H_
#define SIMULATOR_UTILITY_VEHICLE_H_


#include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace framework {


/*
 * Adapter class used to retrieve the id of a node in order to avoid reassigning
 * an address to an already partaking vehicle.
 */
class Vehicle : public veins::TraCICommandInterface::Vehicle {
private:
public:
    std::string getNodeId() { return nodeId; }
    static Vehicle* cast(veins::TraCICommandInterface::Vehicle* vehicle) {
        return reinterpret_cast<Vehicle*>(vehicle);
    }
};

} /* namespace framework */

#endif /* SIMULATOR_UTILITY_VEHICLE_H_ */
