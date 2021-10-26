# Blockchain-based vehicular network simulator
A vehicular netowrk simulator based on Veins for blockchain-based application.

## Compile
The simulator is tested with OMNeT++ 5.3.1, SUMO 1.3.1 and OpenSSL 1.1.

Make sure to setup OMNeT enviornment

```console
cd <omnetpp_dir>
. setenv
```

Compile the simulator

```console
./configure
make
```

## Run a simulation
1. Launch the sumo server
```console
export SUMO\_HOME=<sumo_dir>
cd <project_dir>/veins
./sumo-launchd.py -v -c $SUMO_HOME/bin/sumo
```

2. Set up the simulation scenario in the OMNeT configuration file `omnetpp.in`
in the directory `simulator/examples/simulatori`
3. Execute the run script:
```console
cd <project_dir>/simulator/examples/simulator
./run <opp_run_parameters>
```
For example, to run the MoST traffic scenario in non-graphical mode:
```console
./run -c MoST-MaliciousRandom -u Cmdenv
```
