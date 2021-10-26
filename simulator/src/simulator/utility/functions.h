/*
 * functions.h
 *
 *  Created on: Jun 22, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_UTILITY_FUNCTIONS_H_
#define SIMULATOR_UTILITY_FUNCTIONS_H_

#include <vector>
#include <unordered_set>

#include "veins/base/utils/Coord.h"
#include "simulator/blockchain/Block.h"

namespace framework {

std::vector<unsigned char> sha256(const unsigned char* data, size_t length);
std::vector<unsigned char> sha256(const BlockData& data);
std::vector<unsigned char> bigMult(std::vector<unsigned char> a, long long b);
long long bigLeftShift(std::vector<unsigned char> n, int pos);

bool hashEqual(const Hash& h1, const Hash& h2);



/*template<class ForwardIt, class Compare>
ForwardIt max_element(ForwardIt first, ForwardIt last, Compare comp) {
    if (first == last) return last;

    ForwardIt largest = first;
    ++first;
    for (; first != last; ++first) {
        if (comp(*largest, *first)) {
            largest = first;
        }
    }
    return largest;
}*/

template<class ForwardIt, class Compare>
std::vector<ForwardIt> max_n_element(ForwardIt first, ForwardIt last, int n, Compare comp) {

    std::vector<ForwardIt> elems(n, last);

    size_t pos = 1;
    elems[0] = first;
    ++first;

    for (; first != last; ++first) {
        // find position in result vector
    if(pos < elems.size() || comp(*elems[pos - 1], *first)) {
        auto it = std::lower_bound(elems.begin(), elems.begin() + pos, first,
                       [comp] (ForwardIt a, ForwardIt b) {return comp(*b, *a);});
        // erase lowest element
        elems.erase(elems.begin() + elems.size() - 1);
        elems.insert(it, first);
        if(pos < elems.size())
        pos += 1;
    }
    }
    return elems;
}

/*template<class ForwardIt, class Compare>
ForwardIt min_element(ForwardIt first, ForwardIt last, Compare comp) {
    if (first == last) return last;

    ForwardIt smallest = first;
    ++first;
    for (; first != last; ++first) {
        if (comp(*first, *smallest)) {
            smallest = first;
        }
    }
    return smallest;
}*/

template<class RandomIt, class Compare>
void insertionSort(RandomIt begin, RandomIt end, Compare comp) {
    if(begin == end)
        return;

    for(RandomIt it = begin + 1; it != end; ++it) {
        for(RandomIt elem = it;
                comp(*elem, *(elem - 1)) && elem != begin;
                --elem) {
            auto tmp = *elem;
            *elem = *(elem - 1);
            *(elem - 1) = tmp;
        }
    }
}

template<class T, class U>
void merge(std::map<T, U>& out, std::map<T, U>& in) {
    if(out.empty()) {
        out = std::move(in);
    } else {
        for(auto& it: in) {
            auto r = out.insert(it);
            if(!r.second)
                r.first->second += it.second;
        }
    }
}

template<class T, class U>
void merge(std::unordered_map<T, U>& out, std::unordered_map<T, U>& in) {
    // empty output optimization. Don't move if this means that the output
    // hash table has to shrink
    if(out.empty() && in.bucket_count() >= out.bucket_count()) {
        out = std::move(in);
    } else {
        for(auto& it: in) {
            auto r = out.insert(it);
            if(!r.second)
                r.first->second += it.second;
        }
    }
}

template<class T, class U>
void merge(std::unordered_map<T, std::set<U>>& out, std::unordered_map<T, std::set<U>>& in) {
    // empty output optimization. Don't move if this means that the output
    // hash table has to shrink
    if(out.empty() && in.bucket_count() >= out.bucket_count()) {
        out = std::move(in);
    } else {
        for(auto& it: in) {
            auto r = out.insert(it);
            if(!r.second)
                r.first->second.insert(it.second.begin(), it.second.end());
        }
    }
}

template<class T>
void concat(std::vector<T>& out, std::vector<T>& in) {
    // empty output optimization. Don't move if this means that the output
    // hash table has to shrink
    if(out.empty() && out.capacity() < in.capacity()) {
        out = std::move(in);
    } else {
        out.reserve(out.size() + in.size());
        out.insert(out.end(), in.begin(), in.end());
    }
}

template<class T>
void vectorSum(std::vector<T>& out, std::vector<T> in) {
    if(out.size() < in.size())
        out.resize(in.size());
    for(int i = 0; i < in.size(); ++i)
        out[i] += in[i];
}

}



#endif /* SIMULATOR_UTILITY_FUNCTIONS_H_ */
