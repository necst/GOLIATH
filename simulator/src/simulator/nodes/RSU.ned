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

package org.car2x.veins.subprojects.simulator.nodes;

import org.car2x.veins.base.modules.BaseMobility;
import org.car2x.veins.subprojects.simulator.networking.SimpleInet;
import org.car2x.veins.subprojects.simulator.networking.SimpleWave;
import org.car2x.veins.subprojects.simulator.blockchain.BlockchainManager;
import org.car2x.veins.subprojects.simulator.application.ApplicationLayer;

module RSU
{
    parameters:
    gates:
        input inetIn;
        input dsrcIn;
    submodules:
        inet: SimpleInet;
        wave: SimpleWave;
        app: ApplicationLayer;
        veinsmobility: BaseMobility {
            x = 0.0;
            y = 0.0;
            z = 0.0;
        };
    connections:
        inetIn --> inet.dataIn;
        dsrcIn --> wave.radioIn;
        app.inetSocketIn <-- inet.toApp;
        app.inetSocketOut --> inet.fromApp;
        app.waveSocketIn <-- wave.toApp;
        app.waveSocketOut --> wave.fromApp;
}
