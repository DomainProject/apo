#!/bin/bash

SIMS="1 2"
#RUNS="0 1 2 3 4 5"
RUNS="6 7 8 9"
SOLV="random metis-communication"

respath="res_path"

for i in $SIMS; do
mkdir -p $respath/sim_$i/
	for s in $SOLV; do
		for r in $RUNS; do
			cd ..
			pwd
			python sim_from_trace_test_1.py $s metasimulation/simulation_$i >  experiments/$respath/sim_$i/$s-$r.dat
			cd experiments
		done
	done
done
