import sys


def check_and_install_new_binding(operations, wct_ts):
    binding = operations.delayed_on_window()
    if binding is None: return False
    
    print("got _assignment ")
    
    if binding != sim_state.get_assignment() and sim_state._pending_assigment != binding:
        sim_state.put_pending_assignment(binding)
        print("new binding", binding)
        to_updated = set([])
        for i in range(len(sim_state._assignment)):
            if sim_state._pending_assignment[i] != sim_state._assignment[i]:
                to_updated.add(sim_state._assignment[i])
        for cu in to_updated:
            Simulator.schedule_event(wct_ts, cu, EVT.REASSIGN)
    else:
        print("same binding", binding)
    return True



from metasimulation.SimulationModel.State import State as SimulationState
from metasimulation.window_operations.ddm_operations import DdmOperations

import metasimulation.SimulationEngine.sim as Simulator
from metasimulation.SimulationEngine.sim import *
from metasimulation.SimulationModel.event_handlers import *
from metasimulation.SimulationParameters.global_constants import *


sim_state = SimulationState(sys.argv[1])
sim_state.init_simulator_queue()

# TODO: use sys.argv[1] to change the instantiated object
operations = DdmOperations(sim_state)

rebalance_in_progress = False
rebalance_completed = True
rebalance_cnt = 0


print(f"BEGIN SIMULATION LOOP")

while Simulator.is_there_any_pending_evt() and not sim_state.get_can_end():
    wct_ts, cur_cu, evt_t, a_id, a_ts, actor_from = Simulator.dequeue_event()
    #if wct_ts > 100: 
        #sim_state.set_can_end(True)
        #continue
    
    # Time Window Event
    if evt_t == EVT.TIME_WINDOW:
        communication, annoyance = sim_state.get_state_matrix()

        if wct_ts > 0:


            gvt = sim_state.get_gvt() 
            committed = sim_state.commit()
            print("WT", wct_ts, "GVT", sim_state.get_gvt(), "COM", committed, "TH (com per msec)", committed / time_window_size)

            if not rebalance_in_progress and rebalance_completed:
                rebalance_cnt += 1
                if (rebalance_cnt % rebalance_period) == 0:
                    operations.on_window(
                        sim_state.get_cunits_data(), 
                        wct_ts, 
                        sim_state.get_can_end(), 
                        gvt, 
                        committed,
                        time_window_size, 
                        communication, 
                        annoyance
                    )
                    rebalance_in_progress = True
                    rebalance_completed = False

            sim_state.serialize_stat(wct_ts)

        Simulator.schedule_event(wct_ts + time_window_size, "", EVT.TIME_WINDOW)

        continue

    cu_data = sim_state.get_cunit_data(cur_cu)

    s = f"past wall clock time event {cur_cu}: {wct_ts}, {cu_data['last_wct']} evt: {(wct_ts, cur_cu, evt_t, a_id, a_ts, actor_from)}"
    assert wct_ts >= cu_data['last_wct'], s
    cu_data['last_wct'] = wct_ts

    # the device can start the processing of the highest priority actor
    # schedules the EXE_END event
    if evt_t == EVT.EXE_BGN:
        #this might get empty due to migration  
        if len(cu_data['queue']) > 0: 
            begin_exec(sim_state, cur_cu, cu_data['queue'], cu_data['last_wct'])
        
    # the device has received some task from some other device
    # this do not schedule anything, it just updates task queue of the device
    if evt_t == EVT.RCV:        
        if sim_state._assignment[a_id] != cur_cu:
            print("ON A DIFFERENT DEVICE", cur_cu, sim_state._assignment[a_id])
            Simulator.schedule_event(wct_ts+0.1, sim_state._assignment[a_id], EVT.RCV, (a_id, a_ts, actor_from))
        else:
            recv(sim_state, cu_data['queue'], a_id, a_ts, actor_from, cu_data['last_wct'])

    # the device has completed its non-preemptable execution
    # now check received messages in the meantime
    # if this event is out-of-order,  reschedule this type of event to manage the cost for solving inconstencies
    # if it is in order, schedule EXE_BGN
    if evt_t == EVT.EXE_END:   
        ok = end_exec(sim_state, cur_cu, cu_data['queue'], cu_data['last_wct'])

        if ok  and 'bind' in cu_data and cu_data['bind']:
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
        
    if evt_t == EVT.REASSIGN: cu_data['bind'] = True

    if evt_t == EVT.BIND:
        if len(cu_data['queue']) == 0:
            Simulator.schedule_event(wct_ts, cur_cu, EVT.EXE_BGN)

        heappush(cu_data['queue'], sim_state._actors[a_id])
        cu_data['len'] += 1
        sim_state._assignment[a_id] = cur_cu
        #print("PST", wct_ts, sim_state._assignment)
        if sim_state._assignment == sim_state._pending_assignment and not rebalance_completed:
            print(f"REBALANCE COMPLETED @ {wct_ts} resched @ {wct_ts + time_window_size}")
            #for cu in sim_state.get_cunits_data():
            #    print(cu, sim_state.get_cunits_data()[cu], len(sim_state.get_cunits_data()[cu]['queue']))
            rebalance_completed = True

    # Poll for a rebalancing solution
    if rebalance_in_progress and not rebalance_completed:
        if check_and_install_new_binding(operations, wct_ts):
            rebalance_in_progress = False
            print(f"REBALANCE STARTED @ {wct_ts}")
            #print("PST", wct_ts, sim_state._assignment)

# build application.py for throughput estimator

print(f"END SIMULATION LOOP QUEUE EMPTY {not Simulator.is_there_any_pending_evt()} @ CAN_END {sim_state.get_can_end()} @ WT {wct_ts} @ GVT {sim_state.get_gvt()}")



