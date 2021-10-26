/*
 * Block.h
 *
 *  Created on: May 24, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_BLOCKCHAIN_BLOCK_H_
#define SIMULATOR_BLOCKCHAIN_BLOCK_H_

#include <map>
#include <unordered_map>
#include <memory>
#include <vector>
#include <iostream>

#include "veins/base/utils/SimpleAddress.h"
#include "Transaction.h"
#include "TransactionInfo.h"
#include "NodeStatus.h"

using namespace veins;

namespace framework {

using StatusMap = std::unordered_map<LAddress::L3Type, NodeStatus>;
using Hash = std::vector<unsigned char>;

std::ostream& operator<<(std::ostream& out, const Hash& hash);

class BlockData {
private:
    //use shared_ptr to avoid cloning transactions
    std::vector<std::shared_ptr<const Transaction>> _transactions;
    // no need for ptrs here because transaction-info are computed by a single node
    std::vector<TransactionInfo> infos;
    // use map for this
    StatusMap _statuses;
    Hash _previousHash = Hash(8, 0);
    int height;
    int attempt;

public:
    std::vector<std::shared_ptr<const Transaction>>& transactions();
    const std::vector<std::shared_ptr<const Transaction>>& transactions() const;

    std::vector<TransactionInfo>& transactionInfo();
    const std::vector<TransactionInfo>& transactionInfo() const;

    StatusMap& statuses();
    const StatusMap& statuses() const;

    void previousHash(const Hash& value);
    const Hash& previousHash() const;

    int getHeight() const;
    void setHeight(int height);

    int getAttempt() const;
    void setAttempt(int attempt);

    friend std::vector<unsigned char> sha256(const BlockData& data);

};

class Block {
public:
private:
    int blocksize;
    bool isSigned;
    bool _isLogged = false;

    std::unique_ptr<BlockData> data;
    // don't use BlockData getHeight() otherwise it wouldn't be
    // possible to optimize memory usage by moving the block data
    // instead of copying them
    int height = -1;
    unsigned long long totalTransactions = 0;

    // to be filled on sign, it will only be used to compute the network view
    Hash hash;
    LAddress::L3Type signer;
    // array of supporter nodes
    std::vector<LAddress::L3Type> supporters;

    simtime_t creationTime;

public:

    Block()
        : data {new BlockData()}
    {}

    // block with data
    // obs that block takes ownership of the data
    Block(int blocksize, std::unique_ptr<BlockData>&& data, unsigned long long totalTransactions);

    // obs after signing block can't be modified
    void signBlock(const LAddress::L3Type& signer);
    const LAddress::L3Type& getSigner() const;

    bool isFull() const;

    void addSupporter(const LAddress::L3Type& id);
    const std::vector<LAddress::L3Type>& getSupporters() const;

    const std::vector<std::shared_ptr<const Transaction>>& getAllTransactions() const;
    // shorthand for getAllTransactions()[i]
    std::shared_ptr<const Transaction> getTransaction(size_t index) const;

    const NodeStatus& getStatus(const LAddress::L3Type& id) const;
    const StatusMap& getAllStatuses() const;

    const TransactionInfo& getInfo(size_t index) const;
    const std::vector<TransactionInfo>& getAllInfo() const;

    void markAsLogged();
    bool isLogged() const;

    const Hash& getHash() const;
    const Hash& getPreviousHash() const;

    int getHeight() const;
    unsigned long long getTotalTransactions() const;

    const BlockData& getBlockData() const;

    size_t size() const;
    simtime_t getCreationTime() const;
    int getAttempt() const;
    bool hasData() const;

    friend std::ostream& operator<<(std::ostream& out, const Block& block);
    friend class BlockBuilder;
    friend class OptimizedBlockBuilder;
};


}

#endif /* SIMULATOR_BLOCKCHAIN_BLOCK_H_ */
