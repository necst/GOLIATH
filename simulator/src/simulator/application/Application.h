/*
 * Application.h
 *
 *  Created on: Jul 3, 2020
 *      Author: dvdmff
 */

#ifndef SIMULATOR_APPLICATION_APPLICATION_H_
#define SIMULATOR_APPLICATION_APPLICATION_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace framework {

class Application {
public:
    virtual void sendInetMessage(cMessage* msg) = 0;
    virtual void sendWaveMessage(cMessage* msg) = 0;
};
}


#endif /* SIMULATOR_APPLICATION_APPLICATION_H_ */
