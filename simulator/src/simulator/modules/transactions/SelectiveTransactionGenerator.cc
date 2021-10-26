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

#include "SelectiveTransactionGenerator.h"

Define_Module(framework::SelectiveTransactionGenerator);


namespace framework {

void SelectiveTransactionGenerator::initialize() {
    ValidTransactionGenerator::initialize();
    manager = dynamic_cast<DoubleTypeTraCIManagerLaunchd*>(getModuleByPath("<root>.manager"));
    ASSERT(manager != nullptr);
    addressTable = dynamic_cast<AddressTable*>(getModuleByPath("<root>.addressTable"));
    ASSERT(addressTable != nullptr);
}

void SelectiveTransactionGenerator::handleRequest(NeighbourDiscovery* msg) {
    auto name = addressTable->getNameFor(msg->getSender());
    if(!manager->isMainType(name)) {
        ValidTransactionGenerator::handleRequest(msg);
    } else
        // ignore message
        delete msg;
}

void SelectiveTransactionGenerator::handleReply(NeighbourReply* msg) {
    auto name = addressTable->getNameFor(msg->getSender());
    if(!manager->isMainType(name)) {
        ValidTransactionGenerator::handleReply(msg);
    } else
        // ignore message
        delete msg;
}

}
