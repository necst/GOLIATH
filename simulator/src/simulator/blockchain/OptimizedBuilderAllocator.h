//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __SIMULATOR_OPTIMIZEDBUILDERALLOCATOR_H_
#define __SIMULATOR_OPTIMIZEDBUILDERALLOCATOR_H_

#include <omnetpp.h>

#include "BuilderAllocator.h"

using namespace omnetpp;

namespace framework {
class OptimizedBuilderAllocator : public BuilderAllocator
{
protected:
    std::vector<std::shared_ptr<BlockData>> mainData;
    std::vector<std::shared_ptr<BlockData>> alternativeData;
    int oldestMainData = 0;
    int activeMainData = 0;
    int oldestAlternativeData = 0;
    int activeAlternativeData = 0;
    std::vector<unsigned long long> totalTransactionCounts;

    virtual void initialize() override;
    virtual void finish() override;

public:
    virtual std::shared_ptr<BlockBuilder> getBuilderForHeight(int height, int blocksize,
            TransactionFilter* filter) override;
    virtual std::shared_ptr<BlockBuilder> getAlternativeBuilderForHeight(int height,
            int blocksize, TransactionFilter* filter) override;
    virtual void incrementTransactionCount(int height);
    virtual unsigned long long getTransactionCount(int height);
    virtual void setTransactionCount(int height, unsigned long long count);
};
}
#endif
