/*
 * TransactionFilter.h
 *
 *  Created on: May 25, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_TRANSACTIONFILTER_H_
#define SIMULATOR_BLOCKCHAIN_TRANSACTIONFILTER_H_

#include "Block.h"
#include "Transaction.h"


namespace framework {

struct InsertionInfo;

class TransactionFilter {
public:
    virtual InsertionInfo filter(const BlockData& currentData, const Transaction& tranasction) = 0;
};

struct InsertionInfo {
private:
    InsertionInfo(bool insert, bool overwrite, size_t position)
            : insert {insert},
              overwrite {overwrite},
              position {position}
        {}
public:
    bool insert;
    bool overwrite;
    size_t position;

    InsertionInfo(bool insert)
        : InsertionInfo(insert, false, 0)
    {}

    InsertionInfo(bool overwrite, size_t position)
        : InsertionInfo(false, overwrite, position)
    {}
};

} /* namespace framework */

#endif /* SIMULATOR_BLOCKCHAIN_TRANSACTIONFILTER_H_ */
