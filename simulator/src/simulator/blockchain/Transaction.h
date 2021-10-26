/*
 * Transaction.h
 *
 *  Created on: May 24, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_TRANSACTION_H_
#define SIMULATOR_BLOCKCHAIN_TRANSACTION_H_

#include "veins/base/utils/SimpleAddress.h"
#include "veins/base/utils/Coord.h"

#include <iostream>

using namespace veins;

namespace framework {

class Transaction {
private:
    const LAddress::L3Type sender;
    const LAddress::L3Type target;
    const Coord senderPosition;
    const unsigned range;
    const simtime_t timestamp;
public:
    Transaction(const LAddress::L3Type& sender, const LAddress::L3Type& target, const Coord& senderPosition,
        unsigned range, simtime_t timestamp)
            : sender {sender},
              target {target},
              senderPosition {senderPosition},
              range {range},
              timestamp {timestamp}
    {}

    // Use default copy constructor
    Transaction(const Transaction& other) = default;

    const LAddress::L3Type& getSender() const { return sender; }
    const LAddress::L3Type& getTarget() const { return target; }
    unsigned getRange() const { return range; }
    simtime_t getTimestamp() const { return timestamp; }
    const Coord& getPosition() const { return senderPosition; }

    bool operator<(const Transaction& other) const;

    friend std::ostream& operator<<(std::ostream& out, const Transaction& transaction);
};
using TransactionPtr = std::shared_ptr<const Transaction>;
bool operator<(const TransactionPtr& p1, const TransactionPtr& p2);
}

#endif /* SIMULATOR_BLOCKCHAIN_TRANSACTION_H_ */
