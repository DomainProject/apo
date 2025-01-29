from enum import Enum
from metasimulation.SimulationParameters import global_constants
from metasimulation.SimulationModel import hardware as hardware_model
from metasimulation.SimulationEngine import sim as Simulator
from heapq import *

class EVT(Enum):

    TIME_WINDOW = 0
    REASSIGN= 1
    BIND = 2
    RCV  = 3
    EXE_END = 4
    EXE_BGN = 5
    def __lt__(self, other):
        if self.__class__ is other.__class__:
            return self.value < other.value
        return NotImplemented

def begin_exec(sim_state, cur_cu, queue, last_wct):
    initial = len(queue)
    cur_actor = queue[0]
    vts = cur_actor.curr_ts()
    next_ts = cur_actor.next_ts()

    actor_id = cur_actor.get_id()

    end_exe_wct = last_wct
    end_exe_wct += float(global_constants.task_unit_costs)/hardware_model.get_relative_speed(cur_cu)  # simulate execution


    # just for knowing if all actors have been initiated
    if vts == 0:
        sim_state.add_to_init_set(actor_id)

    # execute all steps of the trace with same ts

    #print("S - PROC", cur_cu, cur_actor, global_constants.task_unit_costs/hardware_model.get_relative_speed(cur_cu),  cur_actor.get_id(), cur_actor.curr_ts(), cur_actor.next_ts())

    while next_ts == cur_actor.next_ts():
        # simulate task send
        if cur_actor.do_step(end_exe_wct):
            a_dest_id = cur_actor.last_trace()[2]
            a_dest_ts = cur_actor.last_trace()[1]
            a_dest_cu = sim_state.get_assignment()[a_dest_id]
            # emulate send cost
            end_exe_wct += hardware_model.get_communication_latency(cur_cu, a_dest_cu)

            Simulator.schedule_event(end_exe_wct, a_dest_cu, EVT.RCV, (a_dest_id, a_dest_ts, actor_id))
        # print("sent",cu_data['last_wct'], a_dest_cu, "RECEIVE", a_dest_id, a_dest_ts, actor_id)

        #print("SENT", cur_cu, global_constants.task_unit_costs/hardware_model.get_relative_speed(cur_cu),  cur_actor.get_id(), cur_actor.curr_ts(), cur_actor.next_ts())

    #print("E - PROC", cur_cu, cur_actor, global_constants.task_unit_costs/hardware_model.get_relative_speed(cur_cu),  cur_actor.get_id(), cur_actor.curr_ts(), cur_actor.next_ts())
    if sim_state.get_can_end() == False and cur_actor.next_ts() == float('inf'):
        print(f"ENDED TRACE FOR {cur_actor}")
        sim_state.set_can_end(True)

    # last_ts_idx[actor_id] = actor_evt_idx
    heapify(queue)
    Simulator.schedule_event(end_exe_wct, cur_cu, EVT.EXE_END)
    if not sim_state.get_can_end(): assert initial == len(queue)


def end_exec(sim_state, cur_cu, queue, last_wct):
    if len(queue) == 0: return
    initial = len(queue)
    heapify(queue)  # resort queue
    
    cur_actor = queue[0] #last_actor_executed[cur_cu]
    a_id = cur_actor.get_id()
    a_ts = cur_actor.next_ts()
    last_exec_ts = cur_actor.curr_ts()

    # received a out-of-order task, hence realign
    end_exe_wct = last_wct
    
    if a_ts < last_exec_ts:
        #print("ROLLING BACK STUFF")
        end_exe_wct += global_constants.task_anno_costs

        #print(f"Managing out of order task for {cur_actor}", a_ts, "vs", last_exec_ts)

        outputs = {}
        #print("FIX EXECUTED", cur_actor._executed[-1], last_exec_ts, a_ts, cur_actor._executed[-5:],  cur_actor.next_ts())
        aborted = cur_actor.fix_executed()
        #print("FIX EXECUTED", cur_actor._executed[-1], last_exec_ts, a_ts, cur_actor._executed[-5:],  cur_actor.next_ts())
        #print("FIX EXECUTED last_executed", cur_actor._executed[-1], "target", a_ts, "next", cur_actor.next_ts(), "curr", cur_actor.curr_ts(), last_exec_ts)
        sim_state._total_aborts[a_id] += aborted
        sim_state._round_aborts[a_id] += aborted
        
        while cur_actor.curr_ts() >= a_ts:
            dst_id = cur_actor.last_trace()[2]
            dst_ts = cur_actor.last_trace()[1]

            if dst_id != a_id:
                if dst_id not in outputs: outputs[dst_id] = float('inf')
                outputs[dst_id] = min(outputs[dst_id], dst_ts)
            cur_actor.do_reverse()
            cur_actor.fix_bound()

        for dst_id in outputs:
            dst_cu = sim_state.get_assignment()[dst_id]
            dst_ts = outputs[dst_id]
            end_exe_wct += hardware_model.get_communication_latency(cur_cu, dst_cu)
            #if sim_state._actors[dst_id].curr_ts() > dst_ts:
            #    print("generating antimessages", dst_id, dst_ts, sim_state._actors[dst_id])
            Simulator.schedule_event(end_exe_wct, dst_cu, EVT.RCV, (dst_id, dst_ts, a_id))

        heapify(queue)
        Simulator.schedule_event(end_exe_wct, cur_cu, EVT.EXE_END)
        return False
    else:
        Simulator.schedule_event(end_exe_wct, cur_cu, EVT.EXE_BGN)
        return True
    if not sim_state.get_can_end(): assert initial == len(queue)


def recv(sim_state, queue, a_id, a_ts, actor_from, last_wct):
    j = -1
    cnt = 0
    for i in range(len(queue)):
        if queue[i].get_id() == a_id:
            j = i
            cnt += 1

    sim_state.update_communication_counters(a_id, actor_from)
    
    if queue[j].curr_ts() >= a_ts:
        #print(f"FUTURE ROLLBACK FROM {actor_from} FOR {a_id} @ {a_ts} {queue[j]} {queue[j]._executed}")
        if actor_from == a_id: 
            print(f"AUTOROLLBACK FROM {actor_from} {a_id}")
            exit(1)
        sim_state.update_annoyance_counters(a_id, actor_from)
        if a_id != actor_from: queue[j].rcv(a_ts, last_wct, actor_from)
    #heapify(queue)  # resort queue
    