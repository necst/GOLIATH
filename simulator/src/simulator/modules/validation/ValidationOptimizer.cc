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

#include "simulator/modules/validation/ValidationOptimizer.h"

Define_Module(framework::ValidationOptimizer);

namespace framework {
void ValidationOptimizer::initialize()
{
    // nothing to do
}

void ValidationOptimizer::finish() {
    result.clear();
}

void ValidationOptimizer::handleMessage(cMessage *msg)
{
    // nothing to do
}

void ValidationOptimizer::cache(int height, int attempt, ValidationResult&& result) {
    this->height = height;
    this->attempt = attempt;
    this->result = result;
}

bool ValidationOptimizer::hasResultFor(int height, int attempt) {
    return this->height == height && this->attempt == attempt;
}

const ValidationResult& ValidationOptimizer::getResultFor(int height, int attempt) {
    if(!hasResultFor(height, attempt))
        throw cRuntimeError("Validation result not in cache");
    return result;
}

}
