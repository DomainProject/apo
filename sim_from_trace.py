#!/usr/bin/env python3

import metasimulation.SimulationEngine.runtime_modules
from metasimulation.SimulationEngine.commandline import *
from metasimulation.SimulationEngine.read_solutions import *

simulation_folder, simulation_trace, fsolutions, wops_string = validate_command_line(sys.argv)

global_constants_parameter_module = metasimulation.SimulationEngine.runtime_modules.global_constants_parameter_module
hardware_parameter_module = metasimulation.SimulationEngine.runtime_modules.hardware_parameter_module

import itertools

from metasimulation.SimulationEngine.assignment import *
from metasimulation.SimulationModel.State import State as SimulationState
from metasimulation.SimulationModel.event_handlers import *

from metasimulation.window_operations.null_operations  import NullOperations

from metasimulation.window_operations.ddm_operations import DdmOperations

from metasimulation.window_operations.metis_hete_asplike import MetisHeterogeneousOperations
from metasimulation.window_operations.metis_hete_comm    import MetisCommunicationOperations
from metasimulation.window_operations.metis_homo_comm  import MetisHomogeneousCommunicationOperations
from metasimulation.window_operations.metis_homo_node  import MetisHomogeneousNodesOperations

from metasimulation.window_operations.random_operations import RandomOperations

import metasimulation.SimulationEngine.simloop


import time
import math


sim_state = SimulationState(simulation_trace, verbose=(len(sys.argv) != 4))
sim_state.init_simulator_queue()

operations_map = {
   "ddm":                     DdmOperations,
   "metis-hete-asplike":   MetisHeterogeneousOperations,
   "random":               RandomOperations,
   "null":                 NullOperations,
   "metis-hete-comm":      MetisCommunicationOperations,
   "metis-homo-comm":      MetisHomogeneousCommunicationOperations,
   "metis-homo-node":      MetisHomogeneousNodesOperations,

}

if wops_string not in operations_map:
    print(f"Invalid operations argument: please select one among {[k for k in operations_map.keys()]}")
    sys.exit(1)

maximum_th, ground_truth = load_ground_truth(fsolutions)
if "ddm" in wops_string and "c" in wops_string:
    operations = operations_map[wops_string](sim_state, ddm_conf=int(wops_string[-1]))
else:
    operations = operations_map[wops_string](sim_state)

evaluate_all = False
to_be_evaluated_assignments = []

if len(sys.argv) == 4:
    evaluate_all = True
    to_be_evaluated_assignments = itertools.product(sim_state.get_cunits_data().keys(), repeat=sim_state.get_num_actors())
    if len(ground_truth) > 0:
        text = input("There is a solutions file. Should overwrite it?[y/n]")
        if text != "y":
            print(f"supplied {text} != y. exiting")
            exit(1)
        else:
            print(f"supplied {text} == y. overwriting")

    fsolutions = open(fsolutions, 'w')
    processes = int(sys.argv[3])
else:
    to_be_evaluated_assignments = [None]
    processes = 1
results = {}

who_am_i = 0
childs = []
if processes > 1:
    print("simulated cases /total cases - skipped by fiter actual expected ETA dd:hh:mm:ss")
    for i in range(processes-1):
        pid = os.fork()
        if pid > 0:
            who_am_i += 1
            childs += [pid]
        else:
            break

rebalance_period = 5
evaluated_tests  = 0
skipped = 0
skipped_par = 0
skipped_fil = 0
start_time = 0
sum_elapsed = 0
all_tests_count = len(sim_state.get_cunits_data().keys())**sim_state.get_num_actors()
# estimated_filter_speedup = estimate_filter_reduction(sim_state.get_cunits_data().keys())
# estimated_filter_skipped = all_tests_count/estimated_filter_speedup
very_start_time = time.time()

for current_assignment in to_be_evaluated_assignments:

    skip = False
    skip_filter = False
    skip_parallel = False
    start_time = time.time()

    if current_assignment != None:
        traces=sim_state._traces

        if (evaluated_tests%processes) != who_am_i: skip_parallel = True
        elif filter_assignment(current_assignment): skip_filter = True
        else:
            Simulator.init_simulation_engine()
            sim_state = SimulationState(sys.argv[2], assignment=list(current_assignment), verbose=False, traces=traces)
            sim_state.init_simulator_queue()
            operations = NullOperations(sim_state)
            rebalance_period = 4
            cur_reb_period = rebalance_period
    else:
        print(f"BEGIN SIMULATION LOOP")

    skip = skip_filter or skip_parallel
    if not skip:
        results[tuple(sim_state.get_assignment())] = []
        wct_ts = metasimulation.SimulationEngine.simloop.loop(sim_state, evaluate_all, maximum_th, ground_truth, rebalance_period, operations, results)

    # build application.py for throughput estimator
    if not evaluate_all:
        print(f"END SIMULATION LOOP QUEUE EMPTY {not Simulator.is_there_any_pending_evt()} @ CAN_END {sim_state.get_can_end()} @ WT {wct_ts} @ GVT {sim_state.get_gvt()}")
        break
    else:
        if not skip:
            assignment = tuple(sim_state.get_assignment())
            res        = results[assignment]
            fsolutions.write(f"{assignment},{sum(res)/len(res)}\n")
            end_time = time.time()
            elapsed = end_time - start_time
            sum_elapsed += elapsed
        else:
            skipped += 1
            if skip_filter: skipped_fil+=1
            if skip_parallel: skipped_par+=1

        remaining = all_tests_count-evaluated_tests
        evaluated_tests += 1

        if skipped != evaluated_tests:
            curr_exec_time = sum_elapsed/(evaluated_tests-skipped)
            eta_max_sec = remaining*curr_exec_time
            eta_seconds = remaining * (curr_exec_time) / (processes * estimated_filter_speedup)
            eta_minutes = math.floor(eta_seconds/60.0)
            eta_seconds -= eta_minutes*60
            eta_hours   = math.floor(eta_minutes/60)
            eta_minutes -= eta_hours*60
            eta_days    = math.floor(eta_hours/24)
            eta_hours   -= eta_days*24

            print(f"\r{evaluated_tests}/{all_tests_count} - {'{:.2f}'.format(skipped_fil * processes /evaluated_tests)} "+
                  f"{'{:.2f}'.format(estimated_filter_skipped/all_tests_count)} ETA {eta_days}d:{eta_hours}h:{eta_minutes}m:{int(eta_seconds)}s "+
                  f"SINGLE TEST {'{:.2f}'.format(curr_exec_time)} ACTUAL USAGE {int(100*sum_elapsed/(time.time()-very_start_time))}% "+" "*20, end='')

        #print(current_assignment, results[current_assignment])

if who_am_i == (processes -1):
    for pid in childs:
        os.waitpid(pid, 0)
