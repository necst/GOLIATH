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

#ifndef __SIMULATOR_OFFSETTRANSACTIONGENERATOR_H_
#define __SIMULATOR_OFFSETTRANSACTIONGENERATOR_H_

#include <omnetpp.h>
#include "MaliciousTransactionGenerator.h"

using namespace omnetpp;

namespace framework {
class OffsetTransactionGenerator : public MaliciousTransactionGenerator
{
protected:
    int minOffset;
    int maxOffset;
    bool changeOffset;
    Coord offset;

    virtual std::vector<std::shared_ptr<const Transaction>> makeTransactions() override;
    virtual Coord getPosition() override;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void setRandomOffset(int minOffset, int maxOffset);
public:
    virtual void handleReply(NeighbourReply* msg) override;
};
}
#endif
