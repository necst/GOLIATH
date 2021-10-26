/*
 * Block.cpp
 *
 *  Created on: May 24, 2020
 *      Author: dvdmff
 */

#include <iomanip>

#include "Block.h"
#include "simulator/utility/functions.h"

namespace framework {

std::ostream& operator<<(std::ostream& out, const Hash& hash) {
    std::ostream hexout(out.rdbuf());
    hexout << std::hex << std::setw(2) << std::setfill('0');
    for(auto v: hash) {
        hexout << (int)v;
    }
    return out;
}

std::vector<std::shared_ptr<const Transaction>>& BlockData::transactions() {
    return _transactions;
}

const std::vector<std::shared_ptr<const Transaction>>& BlockData::transactions() const {
    return _transactions;
}

std::vector<TransactionInfo>& BlockData::transactionInfo() {
    return infos;
}

const std::vector<TransactionInfo>& BlockData::transactionInfo() const {
    return infos;
}

StatusMap& BlockData::statuses() {
    return _statuses;
}

const StatusMap& BlockData::statuses() const {
    return _statuses;
}

void BlockData::previousHash(const Hash& value) {
    _previousHash = value;
}

const Hash& BlockData::previousHash() const {
    return _previousHash;
}

int BlockData::getHeight() const {
    return height;
}

void BlockData::setHeight(int height) {
    this->height = height;
}

int BlockData::getAttempt() const {
    return attempt;
}

void BlockData::setAttempt(int attempt) {
    this->attempt = attempt;
}


Block::Block(int blocksize, std::unique_ptr<BlockData>&& data, unsigned long long totalTransactions)
    : blocksize {blocksize},
      data {std::move(data)},
      totalTransactions {totalTransactions}
{
    height = this->data->getHeight();
}

void Block::signBlock(const LAddress::L3Type& signer) {
    hash = sha256(*data);
    this->signer = signer;
    isSigned = true;
    creationTime = simTime();
}
const LAddress::L3Type& Block::getSigner() const {
    return signer;
}

bool Block::isFull() const {
    return data->transactions().size() >= blocksize;
}

void Block::addSupporter(const LAddress::L3Type& id) {
    supporters.push_back(id);
}

const std::vector<LAddress::L3Type>& Block::getSupporters() const {
    return supporters;
}

const std::vector<std::shared_ptr<const Transaction>>& Block::getAllTransactions() const {
    return data->transactions();
}

std::shared_ptr<const Transaction> Block::getTransaction(size_t index) const {
    return data->transactions().at(index);
}

const NodeStatus& Block::getStatus(const LAddress::L3Type& id) const {
    auto status = data->statuses().find(id);
    if(status == data->statuses().end())
        return INVALID_STATUS;
    return data->statuses()[id];
}

const TransactionInfo& Block::getInfo(size_t index) const {
    return data->transactionInfo().at(index);
}

const std::vector<TransactionInfo>& Block::getAllInfo() const {
    return data->transactionInfo();
}

void Block::markAsLogged() {
    _isLogged = true;
}

bool Block::isLogged() const {
    return _isLogged;
}

const BlockData& Block::getBlockData() const {
    return *data;
}

const Hash& Block::getHash() const {
    ASSERT(isSigned);
    return hash;
}

const Hash& Block::getPreviousHash() const {
    return data->previousHash();
}

size_t Block::size() const {
    return sizeof(Transaction) * data->transactions().size() +
            sizeof(TransactionInfo) * data->transactionInfo().size() +
            sizeof(NodeStatus) * data->statuses().size() +
            sizeof(LAddress::L3Type) * supporters.size();
}

const StatusMap& Block::getAllStatuses() const {
    return data->statuses();
}

simtime_t Block::getCreationTime() const {
    return creationTime;
}

int Block::getHeight() const {
    return height;
}

unsigned long long Block::getTotalTransactions() const {
    return totalTransactions;
}

int Block::getAttempt() const {
    return data->getAttempt();
}

bool Block::hasData() const {
    return (bool)data;
}

std::ostream& operator<<(std::ostream& out, const Block& block){
    out << "Block " << block.getHeight() << " ("
            << block.creationTime << "; attempt " << block.getAttempt() << ")";
    out << "\nHash: " << block.hash;
    out << "\nPrevious hash: " << block.data->previousHash();

    out << "\nSigned by:\n";
    out << "\t" << block.signer << " (harvester)\n";
    for(auto s : block.supporters) {
        out << "\t" << s << "\n";
    }

    out << "Transactions:\n";
    for(int i = 0; i < block.data->transactions().size(); ++i) {
    //for(auto t : block.data->transactions()){
        out << "\t" << *block.data->transactions()[i] << " ["
                << block.data->transactionInfo()[i] << "]\n";
    }
    out << "Statuses:\n";
    for(auto s : block.data->statuses()) {
        out << "\t" << s.first << ": " << s.second << "\n";
    }
    out << "\n";
    return out;
}
}
