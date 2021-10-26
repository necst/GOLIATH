/*
 * TransactionInfo.h
 *
 *  Created on: May 25, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_TRANSACTIONINFO_H_
#define SIMULATOR_BLOCKCHAIN_TRANSACTIONINFO_H_

#include <iostream>

namespace framework {

class TransactionInfo {
private:
    bool isPlausible;
    bool isAcceptable;
    bool isValid;
    std::string reason;
public:
    TransactionInfo(bool plausible, bool acceptable, std::string reason)
        : isPlausible {plausible},
          isAcceptable {acceptable},
          isValid {true},
          reason {reason}
        {}
    TransactionInfo()
        : isPlausible {false},
          isAcceptable {false},
          isValid {false},
          reason {""}
        {}
    bool acceptable() const { return isAcceptable; }
    bool plausible() const { return isPlausible; }
    bool valid() const { return isValid; }

    friend bool operator==(const TransactionInfo& i1, const TransactionInfo& i2);
    friend bool operator!=(const TransactionInfo& i1, const TransactionInfo& i2);

    friend std::ostream& operator<<(std::ostream& out, const TransactionInfo& info);
};

}

#endif /* SIMULATOR_BLOCKCHAIN_TRANSACTIONINFO_H_ */
