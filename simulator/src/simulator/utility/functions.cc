/*
 * functions.cc
 *
 *  Created on: Jun 22, 2020
 *      Author: dvdmff
 */

#include "functions.h"
#include "simulator/blockchain/Block.h"

#include <vector>
#include <stdexcept>

#include <openssl/sha.h>

namespace framework {


std::vector<unsigned char> sha256(const unsigned char* data, size_t length) {
    std::vector<unsigned char> result(SHA256_DIGEST_LENGTH);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, length);
    SHA256_Final(result.data(), &sha256);
    return result;
}

std::vector<unsigned char> sha256(const BlockData& data) {
    //NOTE: the actual content of STL containers may not be hashed properly, is this still ok?
    return sha256(reinterpret_cast<const unsigned char*>(&data), sizeof(BlockData));
}

std::vector<unsigned char> bigMult(std::vector<unsigned char> a, long long b) {
    std::vector<unsigned char> result(sizeof(long long) + a.size(), 0);

    for(int i = 0; i < a.size(); ++i) {
        long long partial = a[i] * b;
        unsigned char carry = 0;
        for(int j = 0; j < sizeof(long long); ++j) {
            unsigned char digit = partial >> (j * sizeof(unsigned char) * 8);
            digit &= 0xff;
            unsigned short tmp = (unsigned short)result[i + j] + (unsigned short)digit;
            result[i + j] = tmp & 0xff;
            carry = tmp >> sizeof(unsigned short) * 8;
        }
        result[i + sizeof(long long) - 1] = carry;
    }
    return result;
}

long long bigLeftShift(std::vector<unsigned char> n, int pos) {
    if(n.size() - pos > sizeof(long long))
        throw std::runtime_error("loosing precision with shift");
    long long result = 0;
    for(int i = pos; i < n.size(); ++i){
        result += n[i] << ((i - pos) * sizeof(unsigned char) * 8);
    }
    return result;
}

bool hashEqual(const Hash& h1, const Hash& h2) {
    return std::equal(h1.begin(), h1.end(), h2.begin());
}

}

