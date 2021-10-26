/*
 * NetworkView.h
 *
 *  Created on: May 26, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_NETWORKVIEW_H_
#define SIMULATOR_BLOCKCHAIN_NETWORKVIEW_H_

#include <vector>

#include "Blockchain.h"

#include "veins/base/utils/SimpleAddress.h"

using namespace veins;

namespace framework {

class Blockchain;

class NetworkView {
private:
    std::vector<LAddress::L3Type> view;
    int harvesters;
    int supporters;
    Hash seed;
    int attempt;

    static std::vector<LAddress::L3Type> hashes2Addresses(std::shared_ptr<Block> block, const std::vector<Hash>& hashes);
public:

    NetworkView() {}
    NetworkView(int harvesters, int supporters, const std::vector<LAddress::L3Type>& view, Hash seed, int attempt);

    static NetworkView compute(Blockchain* blockchain, int harvesters, int supporters, Hash seed, int attempt = 0);

    bool isHarvester(const LAddress::L3Type& id) const;
    bool hasPrecedence(const LAddress::L3Type& id1, const LAddress::L3Type& id2) const;
    bool isSupporter(const LAddress::L3Type& id) const;
    int getHarvesterPosition(const LAddress::L3Type& id) const;
    std::vector<LAddress::L3Type>::const_iterator supportersBegin() const;
    std::vector<LAddress::L3Type>::const_iterator supportersEnd() const;
    std::vector<LAddress::L3Type>::const_iterator harvestersBegin() const;
    std::vector<LAddress::L3Type>::const_iterator harvestersEnd() const;

    bool wasGeneratedFrom(Hash seed, int attempt) const;
    int getAttempt() const;

    friend bool operator==(const NetworkView& v1, const NetworkView& v2);
    friend bool operator!=(const NetworkView& v1, const NetworkView& v2);
};

extern const NetworkView INVALID_VIEW;

} /* namespace framework */

#endif /* SIMULATOR_BLOCKCHAIN_NETWORKVIEW_H_ */
