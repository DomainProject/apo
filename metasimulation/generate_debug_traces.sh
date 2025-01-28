#!/bin/sh

mkdir -p debug_traces
python3 trace_generator.py 10 50000 -1
python3 trace_generator.py 10 50000 10
python3 trace_generator.py 10 50000 100
python3 trace_generator.py 10 50000 500
python3 trace_generator.py 10 100 -1
python3 trace_generator.py 10 100 10
mv *.trace debug_traces
