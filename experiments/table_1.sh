#!/bin/bash

SIMS="1 2"
RUNS="0 1 2 3 4 5 6 7 8 9"
SOLV="random metis-homo-node metis-homo-comm metis-hete-comm" # metis-homogeneous-comm metis-heterogeneous-communication metis-heterogeneous-multilevel" # ddm"

respath="res_path"

for i in $SIMS; do
mkdir -p $respath/sim_$i/
	for s in $SOLV; do
		for r in $RUNS; do
			cd ..
			echo python table1_run.py $s metasimulation/simulation_$i $r
			if [ ! -f experiments/$respath/sim_$i/$s-$r.dat ]; then
				python table1_run.py $s metasimulation/simulation_$i >  experiments/$respath/sim_$i/$s-$r.dat
			fi
			cd experiments
		done
	done
done
