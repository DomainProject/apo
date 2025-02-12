#!/bin/bash

SIMS="1 2 3"
RUNS="0 1 2 3 4 5 6 7 8 9"
SOLV="random metis-homo-node metis-homo-comm metis-hete-comm ddm ddm_c1 ddm_c2 ddm_c3 ddm_c4 ddm_c5 ddm_c6" # metis-homogeneous-comm metis-heterogeneous-communication metis-heterogeneous-multilevel" # ddm"
SOLV="random metis-hete-comm ddm_c6" # metis-homogeneous-comm metis-heterogeneous-communication metis-heterogeneous-multilevel" # ddm"

respath="res_path"

MAX_PARALLEL=10
cnt=0
for i in $SIMS; do
mkdir -p $respath/sim_$i/
	for s in $SOLV; do
		for r in $RUNS; do
			cd ..
			if [ ! -f experiments/$respath/sim_$i/$s-$r.dat ]; then
				echo python table1_run.py $s metasimulation/simulation_$i $r
				python table1_run.py $s metasimulation/simulation_$i >  experiments/$respath/sim_$i/$s-$r.dat &
				(( cnt = (cnt + 1) % ${MAX_PARALLEL} )) || wait
			fi
			cd experiments
		done
	done
done
