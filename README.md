# HOW TO USE THE METASIMULATOR

# Premises

The simulation simulate an infrastructure running a simulation!
Consequently we have 3 different instances of the concept TIME!

1. Wall clock time: the time we experience as humans
2. Meta simulation time: the time exeperienced by the simulated infrastructure (we care for the platform performance)
3. Simulation time: emulates the progress of the application running on the simulated infrastructure

Since we do not care at all of wall-clock time, the Meta-simulation time is called wall-clock time in the source code of the simulator.


# Step 1 get some traces

cd metasimulation && ./generate_debug_traces.sh && cd ..

# Step 2 build clingo stuff

cmake -B cmake-build-debug -S . && cd cmake-build-debug && make && cd ..

# Step 3 run the metasimulator

python sim_from_trace.py <null ddm metis random> metasimulation/debug_traces/<trace>.trace 