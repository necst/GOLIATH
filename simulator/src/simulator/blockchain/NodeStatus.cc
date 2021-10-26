/*
 * NodeStatus.cpp
 *
 *  Created on: May 25, 2020
 *      Author: dvdmff
 */

#include <omnetpp.h>

#include "NodeStatus.h"

using namespace omnetpp;

namespace framework {

NodeStatus::NodeStatus(int reputation, bool active, int maxReputation)
    : reputation { reputation },
      active { active },
      maxReputation {maxReputation},
      bounded {true}
{
    if(this->reputation < 0)
        this->reputation = 0;
    if(this->reputation > maxReputation)
        this->reputation = maxReputation;
}

NodeStatus::NodeStatus(int reputation, bool active)
    : reputation {reputation},
      active {active},
      maxReputation {0},
      bounded {false}
{}

void NodeStatus::uncap() {
    bounded = false;
}

void NodeStatus::cap() {
    bounded = true;
    if(this->reputation < 0)
        this->reputation = 0;
    if(this->reputation > maxReputation)
        this->reputation = maxReputation;
}

void NodeStatus::cap(int maxReputation) {
    if(maxReputation > 0)
        this->maxReputation = maxReputation;
    bounded = true;
    if(this->reputation < 0)
        this->reputation = 0;
    if(this->reputation > this->maxReputation)
        this->reputation = this->maxReputation;
}

void NodeStatus::activate() {
    active = true;
}

void NodeStatus::deactivate() {
    active = false;
}

NodeStatus& NodeStatus::combine(const NodeStatus& other) {
    reputation += other.reputation;
    active |= other.active;
    return *this;
}

NodeStatus& NodeStatus::operator+=(int rhs) {
    reputation += rhs;
    bounded = false;
    return *this;
}

NodeStatus operator+(NodeStatus lhs, int rhs) {
    lhs += rhs;
    return lhs;
}

NodeStatus& NodeStatus::operator-=(int rhs) {
    *this += -rhs;
    return *this;
}

NodeStatus operator-(NodeStatus lhs, int rhs) {
    lhs -= rhs;
    return lhs;
}

NodeStatus& NodeStatus::operator+=(const NodeStatus& rhs) {
    return this->combine(rhs);
}

NodeStatus operator+(NodeStatus lhs, const NodeStatus& rhs) {
    lhs += rhs;
    return lhs;
}

bool operator==(const NodeStatus& lhs, const NodeStatus& rhs) {
    return lhs.reputation == rhs.reputation && lhs.active == rhs.active;
}

bool operator!=(const NodeStatus& lhs, const NodeStatus& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const NodeStatus& status) {
    out << "reputation=" << status.reputation << ", active=" << status.active;
    return out;
}

} /* namespace framework */
