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

#ifndef __SIMULATOR_MIMICTRANSACTIONGENERATOR_H_
#define __SIMULATOR_MIMICTRANSACTIONGENERATOR_H_

#include <omnetpp.h>

#include "MaliciousTransactionGenerator.h"

using namespace omnetpp;

namespace framework {
class MimicTransactionGenerator : public MaliciousTransactionGenerator
{
protected:
    LAddress::L3Type target;
    Hash lastBlock;
    Coord lastTargetPosition;
    bool generateTransactions;
    cMessage* waitTimer;

    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage* msg) override;
    bool isActive(std::shared_ptr<Block> block, LAddress::L3Type target);
    virtual std::vector<std::shared_ptr<const Transaction>> makeTransactions() override;
    virtual Coord getPosition() override;
    bool pickNewTarget();
    bool isTargetInvalid();
};
}
#endif
