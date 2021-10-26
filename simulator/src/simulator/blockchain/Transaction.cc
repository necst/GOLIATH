/*
 * Transaction.cpp
 *
 *  Created on: May 24, 2020
 *      Author: dvdmff
 */

#include "Transaction.h"

namespace framework {


bool Transaction::operator<(const Transaction& other) const {
    if(timestamp < other.timestamp )
        return true;
    /*else if(timestamp == other.timestamp && sender < other.sender)
        return true;
    else if(timestamp == other.timestamp && sender == other.sender && target < other.target)
        return true;*/
    return false;
    //return timestamp < other.timestamp || sender < other.sender || target < other.target;
}

std::ostream& operator<<(std::ostream& out, const Transaction& transaction) {
    out << "from " << transaction.sender << " to " << transaction.target
            << ", range=" << transaction.range << ", timestamp=" << transaction.timestamp
            << ", position=" << transaction.senderPosition;
    return out;
}


bool operator<(const TransactionPtr& p1, const TransactionPtr& p2) {
    return *p1 < *p2;
}

}
