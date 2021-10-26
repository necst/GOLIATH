/*
 * TransactionInfo.cc
 *
 *  Created on: Jul 19, 2020
 *      Author: dvdmff
 */

#include "TransactionInfo.h"

namespace framework {

bool operator==(const TransactionInfo& i1, const TransactionInfo& i2) {
    return i1.acceptable() == i2.acceptable()
            && i1.plausible() == i2.plausible();
}

bool operator!=(const TransactionInfo& i1, const TransactionInfo& i2) {
    return !(i1 == i2);
}

std::ostream& operator<<(std::ostream& out, const TransactionInfo& info) {
    out << "p=" << info.isPlausible << "; a=" << info.isAcceptable << "; " << info.reason;
    return out;
}

}
