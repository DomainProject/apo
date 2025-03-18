#!/bin/bash

simfolder=$1
models="ddm" #metis-hete-asplike metis-hete-comm metis-homo-comm metis-homo-node random"

for m in $models; do
  echo ./sim_from_trace.py $m $simfolder | tee $simfolder/$m.log
done

cd $simfolder
python3 ../../plotta.py metis-hete-asplike.log metis-hete-comm.log metis-homo-comm.log metis-homo-node.log random.log
gnuplot plot.plt
