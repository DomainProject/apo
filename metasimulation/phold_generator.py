#!/bin/python3

import sys
import numpy as np


if len(sys.argv) != 4:
  print("Missing parameters",sys.argv)
  print("CMD should be: cmd < num_actors>0 > < svt_end> <send_prob >")
  exit(1)

num_actors = int(sys.argv[1])
svt_end    = int(sys.argv[2])
send_prob  = float(sys.argv[3])

print(f"Generating trace with:")
print(f"|- #Actors:{num_actors}")
print(f"|- Last TS: {svt_end}")

ofile=f"phold_lp_{num_actors}-end_time_{svt_end}-send_prob_{send_prob}.trace"

print(f"Writing output file '{ofile}'")

actors = []
for i in range(num_actors):
    actors += [[np.random.default_rng(), np.random.default_rng(), np.random.default_rng()]]

f = open(ofile, "w")
f.write(f"src_ts, src_id, dst_ts, dst_id\n")

for a in range(num_actors):
    cur_lvt = 0
    candidates = list(range(num_actors))
    del candidates[a]

    print(f"generating {a}...", end='')
    while cur_lvt < svt_end:
        if actors[a][0].random() < send_prob:
            dest_ts = cur_lvt + actors[a][0].exponential(scale=1.0, size=None)
            dest_lp = actors[a][1].choice(candidates)
            f.write(f"{cur_lvt},{a},{dest_ts},{dest_lp}\n")

        dest_ts = cur_lvt + actors[a][0].exponential(scale=1.0, size=None)
        dest_lp = a
        f.write(f"{cur_lvt},{a},{dest_ts},{dest_lp}\n")
        cur_lvt = dest_ts
    print("done")
f.close()

print("Done")  