import sys
import os
import metasimulation.SimulationEngine.runtime_modules
from metasimulation.SimulationEngine.read_solutions import *

required_files = ["hardware.py", "global_constants.py"]


rebalance_period = 5
cur_reb_period = rebalance_period

if len(sys.argv) < 3:
    print("Usage: python3 sim_from_trace.py <operations> <simulation folder> [parallel processes]")
    print("Valid values for <operations>: ddm, metis, random, stats")
    sys.exit(1)

if not os.path.exists(sys.argv[2]):
    print("Usage: python3 sim_from_trace.py <operations> <simulation folder> [parallel processes]")
    print(f"{simulation_folder} does not exists")
    sys.exit(1)


simulation_folder=sys.argv[2]
simulation_folder_cnts = [f for f in os.listdir(simulation_folder) if os.path.isfile(os.path.join(simulation_folder, f))]
simulation_trace = None
fsolutions = None
wops_string = sys.argv[1]

for f in required_files:
    if f not in simulation_folder_cnts:
        print(f"{simulation_folder} does not contain a '{f}' file")
        sys.exit(1)
    else:
        print(f"{f} found")

cnt=0
for f in simulation_folder_cnts:
    if f[-6:] == '.trace':
        cnt += 1
        simulation_trace = os.path.join(simulation_folder, f)
        fsolutions = simulation_trace+".solutions"

maximum_th = 0
ground_truth = {}
if os.path.isfile(fsolutions):
    ground_truth = load_ground_truth(fsolutions)
    for k in ground_truth:
        maximum_th = max(maximum_th, float(ground_truth[k]))


print(f"trace found ({simulation_trace})")


import importlib.util

file_path = os.path.join(simulation_folder, 'hardware.py')
module_name = 'hardware'
spec = importlib.util.spec_from_file_location(module_name, file_path)
metasimulation.SimulationEngine.runtime_modules.hardware_parameter_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(metasimulation.SimulationEngine.runtime_modules.hardware_parameter_module)
file_path = os.path.join(simulation_folder, 'global_constants.py')
module_name = 'global_constants'
spec = importlib.util.spec_from_file_location(module_name, file_path)
metasimulation.SimulationEngine.runtime_modules.global_constants_parameter_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(metasimulation.SimulationEngine.runtime_modules.global_constants_parameter_module)


global_constants_parameter_module = metasimulation.SimulationEngine.runtime_modules.global_constants_parameter_module
hardware_parameter_module = metasimulation.SimulationEngine.runtime_modules.hardware_parameter_module


import gc
import itertools
from metasimulation.SimulationModel.State import State as SimulationState
from metasimulation.SimulationModel.event_handlers import *
from metasimulation.window_operations.ddm_operations import DdmOperations
from metasimulation.window_operations.metis_operations import MetisOperations
from metasimulation.window_operations.random_operations import RandomOperations
from metasimulation.window_operations.null_operations  import NullOperations
from metasimulation.SimulationModel.hardware import get_dev_from_cu
import time
import math 


def assignment_renaming(assignment):
  new_assignment = []
  translation = {}
  used_labels = set([])
  
  for i in assignment:
    if i not in translation:
      dev = i.split('_')[0]
      count = 0
      while True:
        new_label = f"{dev}_{count}"
        if new_label not in used_labels:
          used_labels.add(new_label)
          translation[i] = new_label
          break
        count += 1
    new_assignment += [translation[i]]
  return new_assignment

def check_and_install_new_binding(operations, wct_ts):
    binding = operations.delayed_on_window()
    if binding is None: return False

    old_bind = binding
    binding = assignment_renaming(binding)
    print(f"new binding: {old_bind} renamed below")
    print(f"new binding: {binding} expected th {ground_truth[tuple(binding)]} vs max th {maximum_th}")
    if binding != sim_state.get_assignment() and sim_state._pending_assigment != binding:
        sim_state.put_pending_assignment(binding)
        to_updated = set([])
        for i in range(len(sim_state._assignment)):
            if sim_state._pending_assignment[i] != sim_state._assignment[i]:
                to_updated.add(sim_state._assignment[i])
        for cu in to_updated:
            Simulator.schedule_event(wct_ts, cu, EVT.REASSIGN)
    return True

def filter_assignment(assignment):
    d = {}
    for cu in assignment:
        dev = get_dev_from_cu(cu)
        if dev not in d: d[dev] = []
        d[dev] += [int(cu.replace(dev+"_", ""))]

    for dev in d:
        used_id = set([])
        order_appearence = []
        for id in d[dev]:
            if id not in used_id: 
                order_appearence += [id]
                used_id.add(id)

        if len(order_appearence) == 1: 
            if order_appearence[0] != 0:
                return True
        else:
            for i in range(len(order_appearence)-1):
                if order_appearence[i] > order_appearence[i+1]: 
                    #print(f"skipping {assignment}")
                    return True
    
    return False

def estimate_filter_reduction(units):
    d = {}
    for cu in units:
        dev = get_dev_from_cu(cu)
        if dev not in d: d[dev] = []
        d[dev] += [int(cu.replace(dev+"_", ""))]
    cnt = 1
    for dev in d:
        tmp = len(d[dev])
        factorial = 1
        while tmp > 1:
            factorial *= tmp
            tmp-=1
        cnt *= factorial
    return cnt



sim_state = SimulationState(simulation_trace, verbose=(len(sys.argv) != 4))
sim_state.init_simulator_queue()

match wops_string:
    case "ddm":
        operations = DdmOperations(sim_state)
    case "metis":
        operations = MetisOperations(sim_state)
    case "random":
        operations = RandomOperations(sim_state)
    case "null":
        operations = NullOperations(sim_state)
    case _:
        print("Invalid operations argument: please select one among 'ddm', 'metis', 'random', 'null'")
        sys.exit(1)

evaluate_all = False
to_be_evaluated_assignments = []

if len(sys.argv) == 4:
    evaluate_all = True
    to_be_evaluated_assignments = itertools.product(sim_state.get_cunits_data().keys(), repeat=sim_state.get_num_actors())
    if len(ground_truth) > 0:
        print("There is a solutions file. Please remove it and then proceed")
        exit(1)
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

evaluated_tests  = 0
skipped = 0
skipped_par = 0
skipped_fil = 0
start_time = 0
sum_elapsed = 0
all_tests_count = len(sim_state.get_cunits_data().keys())**sim_state.get_num_actors()
estimated_filter_speedup = estimate_filter_reduction(sim_state.get_cunits_data().keys()) 
estimated_filter_skipped = all_tests_count/estimated_filter_speedup 
very_start_time = time.time()

for current_assignment in to_be_evaluated_assignments:

    rebalance_in_progress = False
    rebalance_completed = True
    rebalance_cnt = 0
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
            rebalance_period = 2
        
    else:
        print(f"BEGIN SIMULATION LOOP")
    
    skip = skip_filter or skip_parallel
    if not skip: results[tuple(sim_state.get_assignment())] = []
            

    while not skip and Simulator.is_there_any_pending_evt() and not sim_state.get_can_end():
        wct_ts, cur_cu, evt_t, a_id, a_ts, actor_from = Simulator.dequeue_event()
        # Time Window Event
        # just print/collect stats and trigger windows operations
        if evt_t == EVT.TIME_WINDOW:
            communication, annoyance = sim_state.get_state_matrix()

            if wct_ts > 0:

                gvt = sim_state.get_gvt()
                committed = sim_state.commit()
                if not evaluate_all: 
                    sim_state.serialize_stat(wct_ts)
                    truth_th = 0
                    actualth = committed / global_constants_parameter_module.time_window_size
                    if len(ground_truth) > 0:
                        truth_th = float(ground_truth[tuple(sim_state.get_assignment())]) 
                    print("WT", wct_ts, "GVT", sim_state.get_gvt(), "COM", committed, "TH (com per msec)", actualth, "MAX TH", maximum_th)
                    if abs(actualth/truth_th) < 1.05: 
                        cur_reb_period = rebalance_period
                else:
                    results[tuple(sim_state.get_assignment())] += [committed / global_constants_parameter_module.time_window_size]
                if not rebalance_in_progress and rebalance_completed:
                    rebalance_cnt += 1
                    if (rebalance_cnt % cur_reb_period) == 0:
                        operations.on_window(
                            sim_state.get_cunits_data(), 
                            wct_ts, 
                            sim_state.get_can_end(), 
                            gvt, 
                            committed,
                            global_constants_parameter_module.time_window_size,
                            communication,
                            annoyance
                        )
                        rebalance_in_progress = True
                        rebalance_completed = False
                        cur_reb_period = 1000000
                        if evaluate_all == True: sim_state.set_can_end(True)


            Simulator.schedule_event(wct_ts + global_constants_parameter_module.time_window_size, "", EVT.TIME_WINDOW)

            continue

        cu_data = sim_state.get_cunit_data(cur_cu)

        s = f"past wall clock time event {cur_cu}: {wct_ts}, {cu_data['last_wct']} evt: {(wct_ts, cur_cu, evt_t, a_id, a_ts, actor_from)}"
        assert wct_ts >= cu_data['last_wct'], s
        cu_data['last_wct'] = wct_ts

        # the computing unit can start the processing of the highest priority actor
        # if the cu has no actor bound (due to migration) do nothing 
        # otherwise it execute the event and schedules the RCV and EXE_END events
        if evt_t == EVT.EXE_BGN and len(cu_data['queue']) > 0:
            begin_exec(sim_state, cur_cu, cu_data['queue'], cu_data['last_wct'])
            
        # the device has received some task from some other device
        # this do not schedule anything, it just updates task queue of the computing unit and updates related stats
        if evt_t == EVT.RCV:        
            # it might happen that a computing unit received a message for an actor no longer bound to it
            # just forward
            if sim_state._assignment[a_id] != cur_cu:
                #print("ON A DIFFERENT CU", cur_cu, sim_state._assignment[a_id])
                Simulator.schedule_event(wct_ts+0.1, sim_state._assignment[a_id], EVT.RCV, (a_id, a_ts, actor_from))
            else:
                recv(sim_state, cu_data['queue'], a_id, a_ts, actor_from, cu_data['last_wct'])

        # this computing unit has received the request of releasing some actor
        # just set a flag its actual management will be managed during after EXE_END execution
        if evt_t == EVT.REASSIGN: cu_data['bind'] = True

        # the device has completed its non-preemptable execution
        # now check received messages in the meantime
        # if this event is out-of-order,  reschedule EXE_END event to manage the cost for solving inconstencies
        # if it is in order
        #   check if there is reassingment pending
        #         if it is release all its actor and send it to the new computing unit
        #   else 
        #       schedule EXE_BGN

        if evt_t == EVT.EXE_END:   
            scheduled_exe_bgn = end_exec(sim_state, cur_cu, cu_data['queue'], cu_data['last_wct'])

            if scheduled_exe_bgn  and 'bind' in cu_data and cu_data['bind']:
                i = 0
                while i < len(cu_data['queue']):
                    a = cu_data['queue'][i]
                    if sim_state._assignment[a.get_id()] != sim_state._pending_assignment[a.get_id()]:
                        Simulator.schedule_event(wct_ts, sim_state._pending_assignment[a.get_id()], EVT.BIND, (a.get_id(), 0, a.get_id()))
                        del cu_data['queue'][i]
                        cu_data['len'] -= 1
                    else:
                        i+=1
                cu_data['bind'] = False

        # the cu has received an actor so add it to its queue
        # if it was previously empty reschedule a new EXE_BGN
        if evt_t == EVT.BIND:
            if len(cu_data['queue']) == 0:
                Simulator.schedule_event(wct_ts, cur_cu, EVT.EXE_BGN)

            heappush(cu_data['queue'], sim_state._actors[a_id])
            cu_data['len'] += 1
            sim_state._assignment[a_id] = cur_cu
            #print("PST", wct_ts, sim_state._assignment)
            if sim_state._assignment == sim_state._pending_assignment and not rebalance_completed:
                #print(f"REBALANCE COMPLETED @ {wct_ts} resched @ {wct_ts + global_constants_parameter_module.time_window_size}")
                #for cu in sim_state.get_cunits_data():
                #    print(cu, sim_state.get_cunits_data()[cu], len(sim_state.get_cunits_data()[cu]['queue']))
                rebalance_completed = True

        # Poll for a rebalancing solution.
        # This is not associated to an event because it might wait a program running outside the metasimulator
        if rebalance_in_progress and not rebalance_completed:
            if check_and_install_new_binding(operations, wct_ts):
                rebalance_in_progress = False
                #print(f"REBALANCE STARTED @ {wct_ts}")
                #print("PST", wct_ts, sim_state._assignment)

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
            eta_seconds = remaining * (sum_elapsed/(evaluated_tests-skipped)) / (processes * estimated_filter_speedup)
            eta_minutes = math.floor(eta_seconds/60)
            eta_seconds -= eta_minutes*60
            eta_hours   = math.floor(eta_minutes/60)
            eta_minutes -= eta_hours*60
            eta_days    = math.floor(eta_hours/24)
            eta_hours   -= eta_days*24

            print(f"\r{evaluated_tests}/{all_tests_count} - {skipped_fil * processes /evaluated_tests} "+
                  f"{estimated_filter_skipped/all_tests_count} ETA {eta_hours}h:{eta_minutes}m:{int(eta_seconds)}s "+
                  f"SINGLE TEST {sum_elapsed/(evaluated_tests-skipped)} ACTUAL USAGE {int(100*sum_elapsed/(time.time()-very_start_time))}% "+" "*20, end='')

        #print(current_assignment, results[current_assignment]) 

if who_am_i == (processes -1):
    for pid in childs:
        os.waitpid(pid, 0)
