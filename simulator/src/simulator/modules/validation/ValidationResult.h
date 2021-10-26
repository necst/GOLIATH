/*
 * ValidationResult.h
 *
 *  Created on: Aug 5, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_MODULES_VALIDATION_VALIDATIONRESULT_H_
#define SIMULATOR_MODULES_VALIDATION_VALIDATIONRESULT_H_

#include "veins/base/utils/SimpleAddress.h"
#include "simulator/blockchain/Block.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace framework {

using Stats = std::unordered_map<LAddress::L3Type, unsigned>;
using AdjacencyMap = std::unordered_map<LAddress::L3Type, std::set<LAddress::L3Type>>;
//using Cut = std::unordered_map<LAddress::L3Type, std::unordered_set<LAddress::L3Type>>;

class ValidationResult {
public:
    Stats received;
    Stats sent;
    Stats accepted;
    Stats rejected;
    std::vector<TransactionInfo> outcomes;
    AdjacencyMap adjacencyMap;

    ValidationResult() = default;

    void clear();
/*
    ValidationResult(Stats&& targets,
            Stats&& senders,
            StatusMap&& variation,
            std::vector<TransactionInfo>&& outcomes)
        : received {targets},
          sent {senders},
          outcomes {outcomes}
          {}*/
};

void merge(ValidationResult& out, ValidationResult& in);

}

#endif /* SIMULATOR_MODULES_VALIDATION_VALIDATIONRESULT_H_ */
