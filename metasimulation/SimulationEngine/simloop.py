from metasimulation.SimulationEngine import sim as Simulator
from metasimulation.SimulationModel.event_handlers import *
from metasimulation.SimulationEngine.assignment import *
from metasimulation.SimulationEngine.runtime_modules import global_constants_parameter_module

def loop(sim_state, interrupt_early, maximum_th, ground_truth, rebalance_period, operations, results, rebalance_max_cnt=None):
    rebalance_in_progress = False
    rebalance_completed = True
    rebalance_cnt = 0
    cur_reb_period = rebalance_period


    while Simulator.is_there_any_pending_evt() and not sim_state.get_can_end():
            wct_ts, cur_cu, evt_t, a_id, a_ts, actor_from = Simulator.dequeue_event()
            # Time Window Event
            # just print/collect stats and trigger windows operations
            if evt_t == EVT.TIME_WINDOW:
                communication, annoyance = sim_state.get_state_matrix()

                if wct_ts > 0:

                    gvt = sim_state.get_gvt()
                    committed = sim_state.commit()
                    actualth = committed / global_constants_parameter_module.time_window_size
                    
                    if not interrupt_early: 
                        sim_state.serialize_stat(wct_ts)
                        truth_th = 0
                        if len(ground_truth) > 0:
                            truth_th = float(ground_truth[tuple(sim_state.get_assignment())]) 
                        print("WT", wct_ts, "GVT", sim_state.get_gvt(), "COM", committed, "TH (com per msec)", actualth, "MAX TH", maximum_th)
                        if abs(actualth/truth_th) < 1.05: cur_reb_period = rebalance_period
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
                            if interrupt_early == True: sim_state.set_can_end(True)
                            rebalance_in_progress = True
                            rebalance_completed = False
                            cur_reb_period = 1000000
                    if rebalance_max_cnt != None and rebalance_cnt > rebalance_max_cnt: sim_state.set_can_end(True)


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
                if check_and_install_new_binding(operations, wct_ts, maximum_th, ground_truth, sim_state):
                    rebalance_in_progress = False
                    #print(f"REBALANCE STARTED @ {wct_ts}")
                    #print("PST", wct_ts, sim_state._assignment)
    return wct_ts