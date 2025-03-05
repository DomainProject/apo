#!/bin/bash

./sim_from_trace.py ddm metasimulation/$1 | tee ddm.log
./sim_from_trace.py metis-hete-comm metasimulation/$1 | tee metis-hete-comm.log
./sim_from_trace.py metis-homo-comm metasimulation/$1 | tee metis-homo-comm.log
./sim_from_trace.py metis-homo-node metasimulation/$1 | tee metis-homo-node.log
./sim_from_trace.py random metasimulation/$1 | tee random.log

mv ddm.log metasimulation/$1
mv metis-hete-comm.log metasimulation/$1
mv metis-homo-comm.log metasimulation/$1
mv metis-homo-node.log metasimulation/$1
mv random.log metasimulation/$1

cd metasimulation/$1
python3 ../../plotta.py ddm.log metis-hete-comm.log metis-homo-comm.log metis-homo-node.log random.log
gnuplot plot.plt
