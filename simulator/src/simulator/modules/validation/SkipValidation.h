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

#ifndef __SIMULATOR_SKIPVALIDATION_H_
#define __SIMULATOR_SKIPVALIDATION_H_

#include <omnetpp.h>

#include "simulator/blockchain/BlockValidator.h"
#include "simulator/blockchain/Blockchain.h"

using namespace omnetpp;

namespace framework {
class SkipValidation : public cSimpleModule, public BlockValidator
{
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
private:
    TransactionInfo checkTransaction(std::shared_ptr<const Transaction> transaction);
    Blockchain* blockchain;
public:
    void validate(BlockBuilder& builder);
    bool validate(std::shared_ptr<const Block> block);
};
}
#endif
