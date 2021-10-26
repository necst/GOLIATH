/*
 * AddressTable.cc
 *
 *  Created on: Jun 30, 2020
 *      Author: dvdmff
 */

#include "AddressTable.h"

#include <algorithm>

Define_Module(framework::AddressTable);

namespace framework {

void AddressTable::initialize() {
    if(par("logOnAddressAssignment"))
        logger.open(par("logFile").stdstringValue());
}

void AddressTable::handleMessage(cMessage* msg) {
    delete msg;
}

void AddressTable::finish() {
    addresses.clear();
    modules.clear();
    validAddresses.clear();
    if(logger.is_open())
        logger.close();
}

void AddressTable::registerVehicle(std::string vehicleId, cModule* vehicleModule) {
    auto it = addresses.find(vehicleId);
    auto address = LAddress::L3BROADCAST();
    if(it == addresses.end()) {
        // first time joining the network, assign a new address
        ++lastAssignedAddress;
        addresses[vehicleId] = lastAssignedAddress;
        modules[lastAssignedAddress] = std::make_pair(vehicleModule->getFullPath(), vehicleModule);
        address = lastAssignedAddress;
        if(logger.is_open())
            logger << vehicleModule->getFullPath() << " has address " << lastAssignedAddress << std::endl;
    } else {
        // address already assigned, just keep module pointer up-to-date
        modules[it->second] = std::make_pair(vehicleModule->getFullPath(), vehicleModule);
    }
    if(std::find(validAddresses.begin(), validAddresses.end(), address) == validAddresses.end())
        validAddresses.push_back(address);
}

const LAddress::L3Type& AddressTable::getAddress(std::string vehicleId) {
    auto it = addresses.find(vehicleId);
    if(it != addresses.end()) {
        return it->second;
    }
    return LAddress::L3BROADCAST();
}

cModule* AddressTable::getModule(std::string vehicleId) {
    auto it = addresses.find(vehicleId);
    if(it != addresses.end()) {
        auto it2 = modules.find(it->second);
        if(it2 != modules.end())
            return it2->second.second;
    }
    return nullptr;
}

std::string AddressTable::getNameFor(LAddress::L3Type address) {
    auto it = modules.find(address);
    if(it != modules.end())
        return it->second.first;
    return "";
}

const std::unordered_map<LAddress::L3Type, module_description>& AddressTable::getAllModules() {
    return modules;
}

cModule* AddressTable::getModule(LAddress::L3Type address) {
    auto it = modules.find(address);
    if(it != modules.end())
        return it->second.second;
    return nullptr;
}

void AddressTable::leave(LAddress::L3Type address) {
    auto it = modules.find(address);
    if(it != modules.end()) {
        it->second = std::make_pair(it->second.first, nullptr);
    }
    auto pos = std::find(validAddresses.begin(), validAddresses.end(), address);
    if(pos != validAddresses.end())
        validAddresses.erase(pos);
}

const LAddress::L3Type& AddressTable::getRandomAddress() {
    int n = getRNG(0)->intRand(validAddresses.size());
    return validAddresses[n];
}

} /* namespace framework */
