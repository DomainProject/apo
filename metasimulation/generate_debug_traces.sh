#!/bin/sh

# trace generator takes 3 parameters:

# 1: number of actors
# 2: simulated time end
# 3: period in simulated time for sending a potential straggler message (-1 means infinity, so no stragglers)


mkdir -p simulation_1
python3 trace_generator.py 8 500000 -1
mv *.trace simulation_1

mkdir -p simulation_2
python3 trace_generator.py 8 500000 10
mv *.trace simulation_2

mkdir -p simulation_3
python3 phold_generator.py 8 50000 0.5
mv *.trace simulation_3

mkdir -p simulation_4
python3 phold_generator.py 8 50000 0.125
mv *.trace simulation_4
