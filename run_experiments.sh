#!/bin/bash

simfolder=$1
models="random" #metis-hete-asplike metis-hete-comm metis-homo-comm metis-homo-node random"
#models="ddm" #metis-hete-asplike metis-hete-comm"

for m in $models; do
  ./sim_from_trace.py $m $simfolder | tee $simfolder/$m.log
done

cd $simfolder
python3 ../../plotta.py ddm.log metis-hete-asplike.log metis-hete-comm.log random.log #metis-homo-comm.log metis-homo-node.log random.log
gnuplot plot.plt
