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

#ifndef __SIMULATOR_SELECTIVETRANSACTIONGENERATOR_H_
#define __SIMULATOR_SELECTIVETRANSACTIONGENERATOR_H_

#include <omnetpp.h>
#include "simulator/modules/transactions/ValidTransactionGenerator.h"
#include "simulator/mobility/DoubleTypeTraCIManagerLaunchd.h"

using namespace omnetpp;

namespace framework {
class SelectiveTransactionGenerator : public ValidTransactionGenerator
{
protected:
    DoubleTypeTraCIManagerLaunchd* manager;
    AddressTable* addressTable;

    virtual void initialize() override;
public:
    virtual void handleRequest(NeighbourDiscovery* msg) override;
    virtual void handleReply(NeighbourReply* msg) override;
};
}

#endif
