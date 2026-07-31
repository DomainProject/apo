#!/bin/bash

num_actors_list=(16 32 64 128 256 512 1024)

for num_actors in "${num_actors_list[@]}"; do
    python3 imbalanced_generator.py 100 200 200 $num_actors 10 12
    dir=simulation_${num_actors}actors
    mkdir ${dir}
    mv trace_100.0s_200.0b_200.0i_${num_actors}actors_10.0imb_12.0rate.trace ${dir}
    cp global_constants.py ${dir}
    cp hardware.py ${dir}
done

num_units_list=(1 2 4 8 16 32)

mkdir -p hardware_scaling

python3 imbalanced_generator.py 100 200 200 32 10 12
trace_file="trace_100.0s_200.0b_200.0i_32actors_10.0imb_12.0rate.trace"

for x in "${num_units_list[@]}"; do
for y in "${num_units_list[@]}"; do
for z in "${num_units_list[@]}"; do
    dir="hardware_scaling/simulation_${x}CPUs_${y}GPUs_${z}FPGAs"
    mkdir -p "$dir"
    cp "$trace_file" "$dir"
    cp global_constants.py "$dir"
    cat > "$dir/hardware.py" << EOF
cu_types = {
'cpu' : {
    'num_units':${x},
    'relative_speed':1.0,
    },
'gpu' : {'num_units':${y}, 'relative_speed':4},
'fpga': {'num_units':${z}, 'relative_speed':2},
}

comm_unitary_cost =  0.020

communication_costs = {}
for k1 in cu_types:
  communication_costs[k1] = {}
  for k2 in cu_types:
    communication_costs[k1][k2] = 1
    if k1 == 'gpu'  and k2 == 'cpu': communication_costs[k1][k2] = 5
    if k2 == 'gpu'  and k1 == 'cpu': communication_costs[k1][k2] = 5
    if k1 == 'fpga' and k2 == 'cpu': communication_costs[k1][k2] = 10
    if k2 == 'fpga' and k1 == 'cpu': communication_costs[k1][k2] = 10
    if k1 == 'fpga' and k2 == 'gpu': communication_costs[k1][k2] = 20
    if k2 == 'fpga' and k1 == 'gpu': communication_costs[k1][k2] = 20
EOF
done
done
done

rm "$trace_file"

echo "All simulation traces generated"