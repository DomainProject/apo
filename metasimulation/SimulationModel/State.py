#from metasimulation.SimulationParameters.global_constants import *
#from metasimulation.SimulationParameters.hardware import *


from metasimulation.SimulationEngine.runtime_modules import global_constants_parameter_module as global_constants
from metasimulation.SimulationEngine.runtime_modules import hardware_parameter_module as hardware_constants

import metasimulation.SimulationModel.hardware as hardware
from metasimulation.SimulationModel.actor import Actor as Actor
from metasimulation.SimulationEngine.sim import *
from metasimulation.SimulationModel.event_handlers import EVT

from heapq import *


class State():
    _verbose = verbose
    _trace_path = []
    _traces = []
    _annoyance_matrix = []
    _communication_matrix = []
    _init_set = set([])
    _num_actors = 0
    _cu_units_data = {}
    _assignment = []
    _can_end = False
    _previous_commits = None
    _round_aborts = []
    _total_aborts = []
    _actors = []
    _pending_assigment = []
    _gvt = -1-0

    def __init__(self, path, assignment=None, verbose=True, traces=None):
        self._verbose = verbose
        if self._verbose: print(f"Loading trace {path}...", end='')
        self._trace_path = path
        if traces != None:
            self._traces = traces
        else:
            self._traces = load_trace(path)

        if self._verbose: print("Done")

        self._num_actors = len(self._traces)

        if self._verbose: print(f"Building annoyance_matrix and communication_matrix for {self._num_actors} actors...", end='')
        for i in range(self._num_actors):
            self._annoyance_matrix += [[]]
            self._communication_matrix += [[]]
            self._total_aborts += [0]
            self._round_aborts += [0]
            for j in range(self._num_actors):
                self._annoyance_matrix[i] += [0]
                self._communication_matrix[i] += [0]
        if self._verbose: print("Done")


        if self._verbose: print(f"Loading hardware configuration...", end='')
        cu_units = sorted(hardware.build_cunits())
        if self._verbose: print("Done")

<<<<<<< HEAD
        if self._verbose: print(f"Building computing units...", end='')
        for k in cu_units:    
=======
        print(f"Building computing units...", end='')
        for k in cu_units:
>>>>>>> metis-integration-2
            self._cu_units_data[k] = {'last_wct': 0, "queue": [], "len": 0}
        if self._verbose: print("Done")

        if self._verbose: print(f"Building assignment...", end='')
        if assignment == None:
            cnt = 0
            for k in range(self._num_actors):
                self._assignment += [cu_units[cnt]]
                cnt = (cnt + 1) % len(cu_units)
            self._assignment = ['cpu_0'] * self._num_actors #, 'cpu_0', 'cpu_0', 'cpu_0', 'cpu_0', 'cpu_0', 'cpu_0', 'cpu_0', 'cpu_0', 'cpu_0']
            if self._verbose: print(self._assignment)
        else:
            self._assignment = assignment
        if self._verbose: print(f"done")

        if self._verbose: print(f"Populate device queue with new actors...", end='')
        for a_id in range(len(self._assignment)):
            a = Actor(a_id, self._traces[a_id])
            self._actors += [a]
            heappush(self._cu_units_data[self._assignment[a_id]]['queue'], a)
            self._cu_units_data[self._assignment[a_id]]['len'] += 1
        if self._verbose: print(f"done")


    def put_pending_assignment(self, assignment):
        self._pending_assignment = assignment

    def init_simulator_queue(self):
        if self._verbose: print(f"Populate simulator queue...", end='')
        for cu in self._cu_units_data:
            schedule_event(0, cu, EVT.EXE_BGN)
        schedule_event(0, "", EVT.TIME_WINDOW)
        if self._verbose: print(f"done")

    def get_cunits(self):     return sorted(self._cu_units_data.keys())

    def get_num_actors(self): return self._num_actors

    def get_cunit_data(self, cur_cu):  return self._cu_units_data[cur_cu]
    def get_cunits_data(self):         return self._cu_units_data

    def add_to_init_set(self, aid):
        self._init_set.add(aid)
        #if len(self._init_set) == self._num_actors: print("END ACTOR INIT")

    def get_assignment(self): return self._assignment

    def update_communication_counters(self, aid, bid): self._communication_matrix[aid][bid] += 1
    def update_annoyance_counters(self, aid, bid): self._annoyance_matrix[aid][bid] += 1

    def get_state_matrix(self): return self._communication_matrix, self._annoyance_matrix

    def set_can_end(self, can_end): self._can_end = can_end
    def get_can_end(self):          return self._can_end

    def get_actor_by_id(self, aid): return self._actors[aid]
    def get_gvt(self):

        gvt = float('inf')
        for cu in self._cu_units_data:
            for a in self._cu_units_data[cu]['queue']:
                gvt = min(gvt, a.curr_ts())
                gvt = min(gvt, a.next_ts())
        self._gvt = gvt
        return gvt

    def commit(self):

        cmt_dict = {}
        for cu in self._cu_units_data:
            for a in self._cu_units_data[cu]['queue']:
                cmt_dict[a.get_id()] = a.commit(self._gvt)

        cnt = []

        for a in sorted(cmt_dict.keys()):
            cnt += [cmt_dict[a]]
        #print("EVT COMMITTED ROUND", cnt)

        min_cmt = min(cnt)
        max_cmt = max(cnt)
        if min_cmt != max_cmt and False:
            print(self._gvt)
            for cu in self._cu_units_data:
                for a in self._cu_units_data[cu]['queue']:
                    print(a, a._executed)
            exit(1)
        if self._previous_commits == None:
            self._previous_commits = cnt
        else:
            for i in range(len(cnt)):
                self._previous_commits[i] += cnt[i]
            #print("EVT COMMITTED TOTAL", self._previous_commits)

        #print("EVT ABORTED ROUND", self._round_aborts)
        for i in range(len(self._round_aborts)):
            self._round_aborts[i] = 0
        #print("EVT ABORTED TOTAL", self._total_aborts)



        return sum(cnt)


    def serialize_stat(self, wct_ts):
        comm = []
        anno_matrix = []

        for i in range(self._num_actors):
            comm += [[]]
            anno_matrix += [[]]
            for j in range(self._num_actors):
                comm[i] += [0]
                anno_matrix[i] += [0]

        for i in range(self._num_actors):
            for j in range(self._num_actors):
                comm[i][j] = self._communication_matrix[i][j]/wct_ts
                anno_matrix[i][j] = self._annoyance_matrix[i][j]/wct_ts

        f = open('application.py', 'w')
        f.write(f"num_actors        = {self._num_actors}       \n")
        f.write(f"task_unit_costs   = {global_constants.task_unit_costs}  \n")
        f.write(f"task_anno_costs   = {global_constants.task_anno_costs}  \n")
        f.write(f"comm_unitary_cost = {hardware_constants.comm_unitary_cost}    \n")

        f.write(f"comm_matrix = [\n")
        for i in range(self._num_actors):
            f.write(f"{comm[i]}")
            if i != self._num_actors - 1:        f.write(f",")
            f.write(f"\n")
        f.write(f"]\n")

        f.write(f"anno_matrix      = [\n")
        for i in range(self._num_actors):
            f.write(f"{anno_matrix[i]}")
            if i != self._num_actors - 1:        f.write(f",")
            f.write(f"\n")
        f.write(f"]\n")




def load_trace(path):
    if path[-6:] != ".trace":
        print("the filename does not ends with '.trace'")
        exit(1)

    f = open(path)
    traces = {}
    cnt=0
    for line in f.readlines():
        if cnt == 0 and ',' in line:
            cnt+=1
            continue
        if cnt != 0 and ',' in line:
            line = line.strip().split(',')
            if int(line[1]) not in traces: traces[int(line[1])] = []
            #print(line)
            traces[int(line[1])] += [ ( float(line[0]), float(line[2]), int(line[3]) ) ]
    f.close()
    for k in traces:
        traces[k] = sorted(traces[k], key=lambda x:x[0])
    return traces
