/*
 * NetworkView.cc
 *
 *  Created on: May 26, 2020
 *      Author: dvdmff
 */

#include "NetworkView.h"
#include "Block.h"

#include "simulator/utility/functions.h"

#include <vector>
#include <algorithm>

namespace framework {

const NetworkView INVALID_VIEW = NetworkView(-1, -1, {}, {}, -1);

NetworkView NetworkView::compute(Blockchain* blockchain, int harvesters, int supporters, Hash seed, int attempt) {
    int total = harvesters + supporters;

    std::vector<Hash> hashes;
    Hash completeSeed(seed);
    for(int i = 0; i < sizeof(attempt) / sizeof(unsigned char); ++i) {
        completeSeed.push_back(attempt >> (i * sizeof(unsigned char) * 8));
    }

    for(int i = 0; i < sizeof(int) / sizeof(unsigned char); ++i) {
	completeSeed.push_back(0);
    }

    for(int i = 0; i < total; ++i) {
        auto newHash = sha256(completeSeed.data(), sizeof(unsigned char) * completeSeed.size());
        hashes.push_back(newHash);
	//update index number
	unsigned char carry = 1;
	for(int j = 0; j < sizeof(int) / sizeof(unsigned char) && carry == 1; ++j) {
	    auto size = completeSeed.size();
	    completeSeed[size - 1 - j] += carry;
	    if(completeSeed[size - 1 - j] != 0)
		carry = 0;
	}
    }

    // if the blockchain work properly, i should be 0 or 1
    int i = 0;
    auto block = blockchain->getNthMostRecentBlock(i);
    while(block && !hashEqual(block->getHash(), seed)) {
        ++i;
        block = blockchain->getNthMostRecentBlock(i);
    }
    // block not found, cannot build the view
    if(!block)
        return INVALID_VIEW;

    return NetworkView(harvesters, supporters, hashes2Addresses(blockchain->getNthMostRecentBlock(i), hashes), seed, attempt);
}

std::vector<LAddress::L3Type> NetworkView::hashes2Addresses(std::shared_ptr<Block> block, const std::vector<Hash>& hashes) {

    std::vector<LAddress::L3Type> nodes {};
    std::vector<long long> ranges {};

    long long total = 0;
    for(auto e : block->getAllStatuses()) {
        total += e.second.reputation * (int)e.second.active;
        nodes.push_back(e.first);
        ranges.push_back(total);
    }

    std::vector<LAddress::L3Type> addresses;

    for(auto h : hashes) {
        auto product = bigMult(h, total);
        auto value = bigLeftShift(product, h.size());

        auto it = std::lower_bound(ranges.begin(), ranges.end(), value);
        addresses.push_back(nodes[it - ranges.begin()]);
    }

    return addresses;

}

NetworkView::NetworkView(int harvesters, int supporters,
        const std::vector<LAddress::L3Type>& view,
        Hash seed, int attempt)
    : view {view},
      harvesters {harvesters},
      supporters {supporters},
      seed {seed},
      attempt {attempt}
{ }


bool NetworkView::isHarvester(const LAddress::L3Type& id) const {
    auto it = std::find(harvestersBegin(), harvestersEnd(), id);
    if(it == harvestersEnd())
        return false;
    return true;
}

bool NetworkView::hasPrecedence(const LAddress::L3Type& id1, const LAddress::L3Type& id2) const {
    auto it1 = std::find(view.begin(), view.end(), id1);
    auto it2 = std::find(view.begin(), view.end(), id2);
    //use lexicographical order
    return std::distance(view.begin(), it1) < std::distance(view.begin(), it2);
}

int NetworkView::getHarvesterPosition(const LAddress::L3Type& id) const {
    auto it = std::find(harvestersBegin(), harvestersEnd(), id);
    if(it != harvestersEnd())
        return it - harvestersBegin();
    return -1;
}

bool NetworkView::isSupporter(const LAddress::L3Type& id) const {
    auto it = std::find(supportersBegin(), supportersEnd(), id);
    if(it == supportersEnd())
        return false;
    return true;
}

bool NetworkView::wasGeneratedFrom(Hash seed, int attempt) const {
    return seed.size() == this->seed.size()
            && std::equal(seed.begin(), seed.end(), this->seed.begin())
            && attempt == this->attempt;
}

int NetworkView::getAttempt() const {
    return attempt;
}

std::vector<LAddress::L3Type>::const_iterator NetworkView::supportersBegin() const {
    return view.begin() + harvesters;
}

std::vector<LAddress::L3Type>::const_iterator NetworkView::supportersEnd() const {
    return view.end();
}

std::vector<LAddress::L3Type>::const_iterator NetworkView::harvestersBegin() const {
    return view.begin();
}

std::vector<LAddress::L3Type>::const_iterator NetworkView::harvestersEnd() const {
    return view.begin() + harvesters;
}

bool operator==(const NetworkView& v1, const NetworkView& v2) {
    return v1.view.size() == v2.view.size()
            && std::equal(v1.view.begin(), v1.view.end(), v2.view.begin())
            && v1.harvesters == v2.harvesters && v1.supporters == v2.supporters;
}

bool operator!=(const NetworkView& v1, const NetworkView& v2) {
    return !(v1 == v2);
}

} /* namespace framework */
