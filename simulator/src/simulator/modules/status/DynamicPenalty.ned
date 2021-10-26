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

package org.car2x.veins.subprojects.simulator.modules.status;

import org.car2x.veins.subprojects.simulator.blockchain.IStatusUpdater;

simple DynamicPenalty like IStatusUpdater
{
    parameters:
        @class(framework::DynamicPenalty);
        int reward = default(256);
        int penalty = default(256);
        int blockReward = default(256);
        int blockPenalty = default(256);
        int startingReward = default(1024);
        int rsuStartingReward = default(2048);
        int maxReward = default(4096);
        
        int scaleFactor = default(2);
}
