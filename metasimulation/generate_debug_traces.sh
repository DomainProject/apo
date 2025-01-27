#!/bin/sh

mkdir -p debug_traces
python3 trace_generator.py 10 50000 -1
python3 trace_generator.py 10 50000 100
mv *.trace debug_traces
