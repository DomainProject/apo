#!/bin/python3

import sys

if len(sys.argv) != 4:
  print("Missing parameters",sys.argv)
  print("CMD should be: cmd < num_actors>0 > < svt_end>0 > < fan_out_period!=0 (default <0=infty) >")
  exit(1)

num_actors = int(sys.argv[1])
svt_end    = int(sys.argv[2])
fan_out_per= int(sys.argv[3]) 

print(f"Generating trace with:")
print(f"|- #Actors:{num_actors}")
if fan_out_per == 0:
  fan_out_per = -1
if fan_out_per < 0:
  print(f"|- 1 Outbound EVT Period: {float('inf')}")
else:
  print(f"|- 1 Outbound EVT Period: {fan_out_per}")
print(f"|- Last TS: {svt_end}")

ofile=f"lp_{num_actors}-end_time_{svt_end}-fanout_{fan_out_per}.trace"

print(f"Writing output file '{ofile}'")

f = open(ofile, "w")
f.write(f"src_ts, src_id, dst_ts, dst_id\n")
for i in range(1,svt_end):
  for a in range(num_actors):
    f.write(f"{i},{a},{i+1},{a}\n")
    if (i % fan_out_per) == 0 and fan_out_per > 0:
      f.write(f"{i},{a},{i+0.1},{(a+1)%num_actors}\n")
f.close()

print("Done")  