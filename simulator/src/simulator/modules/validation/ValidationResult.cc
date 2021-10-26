/*
 * MaliciousTransactionGenerator.cc
 *
 *  Created on: Sep 29, 2020
 *      Author: dvdmff
 */

#include "ValidationResult.h"
#include "simulator/utility/functions.h"

namespace framework {

void ValidationResult::clear() {
    sent.clear();
    received.clear();
    accepted.clear();
    rejected.clear();
    outcomes.clear();
}

void merge(ValidationResult& out, ValidationResult& in) {
    merge(out.sent, in.sent);
    merge(out.received, in.received);
    merge(out.accepted, in.accepted);
    merge(out.rejected, in.rejected);

}


}
