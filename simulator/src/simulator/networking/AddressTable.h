/*
 * AddressTable.h
 *
 *  Created on: Jun 30, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_NETWORKING_ADDRESSTABLE_H_
#define SIMULATOR_NETWORKING_ADDRESSTABLE_H_

#include <string>
#include <map>
#include <unordered_map>
#include <fstream>

#include <omnetpp.h>

#include "veins/base/utils/SimpleAddress.h"

using namespace veins;
using namespace omnetpp;

namespace framework {

using module_description = std::pair<std::string, cModule*>;
class AddressTable : public cSimpleModule {
private:
    LAddress::L3Type lastAssignedAddress = -1;
    std::unordered_map<std::string, LAddress::L3Type> addresses;
    std::unordered_map<LAddress::L3Type, module_description> modules;
    std::vector<LAddress::L3Type> validAddresses;
    std::ofstream logger;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage* msg);
    virtual void finish();
public:
    const LAddress::L3Type& getAddress(std::string vehicleId);
    cModule* getModule(std::string vehicleId);
    cModule* getModule(LAddress::L3Type address);
    std::string getNameFor(LAddress::L3Type address);
    void registerVehicle(std::string vehicleId, cModule* vehicleModule);
    void leave(LAddress::L3Type address);
    const std::unordered_map<LAddress::L3Type, module_description>& getAllModules();
    const LAddress::L3Type& getRandomAddress();
};

} /* namespace framework */

#endif /* SIMULATOR_NETWORKING_ADDRESSTABLE_H_ */
