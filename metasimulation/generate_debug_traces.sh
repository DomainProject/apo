#!/bin/sh

# trace generator takes 3 parameters:

# 1: number of actors
# 2: simulated time end
# 3: period in simulated time for sending a potential straggler message (-1 means infinity, so no stragglers)

mkdir -p debug_traces
python3 trace_generator.py 10 50000 -1
python3 trace_generator.py 10 50000 10
python3 trace_generator.py 10 50000 100
python3 trace_generator.py 10 50000 500
python3 trace_generator.py 10 100 -1
python3 trace_generator.py 10 100 10
mv *.trace debug_traces
