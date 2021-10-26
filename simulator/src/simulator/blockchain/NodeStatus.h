/*
 * NodeStatus.h
 *
 *  Created on: May 25, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_NODESTATUS_H_
#define SIMULATOR_BLOCKCHAIN_NODESTATUS_H_

#include <iostream>

namespace framework {

class NodeStatus {
public:
    int reputation = 0;
    bool active = false;
    int maxReputation = 0;
    bool bounded = false;
public:
    NodeStatus() {}
    NodeStatus(int reputation, bool active, int maxReputation);
    NodeStatus(int reputation, bool active);

    void activate();
    void deactivate();
    void uncap();
    void cap();
    void cap(int maxReputation);
    NodeStatus& combine(const NodeStatus& other);
    // A binary operation to combine two node statuses
    NodeStatus& operator+=(const NodeStatus& rhs);
    friend NodeStatus operator+(NodeStatus lhs, const NodeStatus& rhs);
    NodeStatus& operator+=(int rhs);
    friend NodeStatus operator+(NodeStatus lhs, int rhs);
    NodeStatus& operator-=(int rhs);
    friend NodeStatus operator-(NodeStatus lhs, int rhs);
    friend bool operator==(const NodeStatus& lhs, const NodeStatus& rhs);
    friend bool operator!=(const NodeStatus& lhs, const NodeStatus& rhs);

    friend std::ostream& operator<<(std::ostream& out, const NodeStatus& status);
};

static const NodeStatus INVALID_STATUS = NodeStatus(-1, false, -1);
} /* namespace framework */

#endif /* SIMULATOR_BLOCKCHAIN_NODESTATUS_H_ */
