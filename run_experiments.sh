#!/bin/bash

#./sim_from_trace.py ddm metasimulation/$1 | tee ddm.log
#./sim_from_trace.py metis-hete-asplike metasimulation/$1 | tee metis-hete-asplike.log
#./sim_from_trace.py metis-hete-comm metasimulation/$1 | tee metis-hete-comm.log
#./sim_from_trace.py metis-homo-comm metasimulation/$1 | tee metis-homo-comm.log
#./sim_from_trace.py metis-homo-node metasimulation/$1 | tee metis-homo-node.log
#./sim_from_trace.py random metasimulation/$1 | tee random.log
#
#mv ddm.log metasimulation/$1
#mv metis-hete-asplike.log metasimulation/$1
#mv metis-hete-comm.log metasimulation/$1
#mv metis-homo-comm.log metasimulation/$1
#mv metis-homo-node.log metasimulation/$1
#mv random.log metasimulation/$1
#
#cd metasimulation/$1
#python3 ../../plotta.py ddm.log metis-hete-asplike.log metis-hete-comm.log metis-homo-comm.log metis-homo-node.log random.log
#gnuplot plot.plt


RUN_MODE=${1:-"both"}  # "actors", "hardware", or "both"

run_actors() {
    num_actors_list=(16 32 64 128 256 512 1024)

    output_csv="results.csv"
    echo "num_actors,time,time_ddm" > $output_csv

    for num_actors in "${num_actors_list[@]}"; do
        start=$(date +%s%N)
        output=$(python3 sim_from_trace.py ddm metasimulation/simulation_${num_actors}actors | tee /dev/tty)
        end=$(date +%s%N)

        time_total=$(echo "scale=6; ($end - $start) / 1000000000" | bc)
        time_ddm=$(echo "$output" | grep "DDM_TIME:" | cut -d: -f2)

        echo "$num_actors,$time_total,$time_ddm" >> $output_csv
        echo "  → Execution time (${num_actors} actors): ${time_total}s  |  DDM: ${time_ddm}s"
    done
}

run_hardware() {
    output_csv="results_hardware.csv"
    echo "num_cpus,num_gpus,num_fpgas,time,time_ddm" > $output_csv

    num_units_list=(1 2 4 8)

    for x in "${num_units_list[@]}"; do
    for y in "${num_units_list[@]}"; do
    for z in "${num_units_list[@]}"; do
        dir="metasimulation/hardware_scaling/simulation_${x}CPUs_${y}GPUs_${z}FPGAs"

        start=$(date +%s%N)

        output=$(timeout 300 python3 sim_from_trace.py ddm "$dir" | tee /dev/tty)
        exit_code=${PIPESTATUS[0]}

        if [ $exit_code -eq 124 ]; then
            echo "  → TIMEOUT (${x}CPUs ${y}GPUs ${z}FPGAs), skipping"
            echo "$x,$y,$z,timeout,timeout" >> $output_csv
            continue
        fi

        end=$(date +%s%N)

        time_total=$(echo "scale=6; ($end - $start) / 1000000000" | bc)
        time_ddm=$(echo "$output" | grep "DDM_TIME:" | cut -d: -f2)

        echo "$x,$y,$z,$time_total,$time_ddm" >> $output_csv
        echo "  → Execution time (${x}CPUs ${y}GPUs ${z}FPGAs): ${time_total}s  |  DDM: ${time_ddm}s"
    done
    done
    done
}

case "$RUN_MODE" in
    actors)
        run_actors
        ;;
    hardware)
        run_hardware
        ;;
    both)
        run_actors
        run_hardware
        ;;
    *)
        echo "Uso: $0 [actors|hardware|both]"
        exit 1
        ;;
esac