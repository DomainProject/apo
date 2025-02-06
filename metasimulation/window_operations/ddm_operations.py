import math

from metasimulation.SimulationModel.hardware import convert_ddm_assignment_to_sim_assingment, get_relative_speed
from metasimulation.window_operations.abstract_operations import WindowOperations
from src.ddm import ddm_init, ddm_optimize, ddm_prepare_actor_matrix, ddm_poll

from metasimulation.SimulationModel.hardware import get_communication_latency, get_relative_speed
from metasimulation.SimulationEngine.runtime_modules import hardware_parameter_module as hardware_constants


class DdmOperations(WindowOperations):

    def __init__(self, sim_state, ddm_conf=0):
        print(f"initialize DDM...", end='')
        self.sim_state = sim_state
        cunits = sim_state.get_cunits()
        cus = [{'cpu': 1, 'gpu': 2, 'fpga': 4}[x.split('_')[0]] for x in cunits]
        msg_exch_cost = [ [ int(get_communication_latency(x,y)/hardware_constants.comm_unitary_cost) for y in cunits] for x in cunits]
        runnable_on = [7] * sim_state.get_num_actors()
        ddm_init(len(cus), sim_state.get_num_actors(), cus, msg_exch_cost, runnable_on, conf=ddm_conf)
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, min_vt, committed, time_window_size,
                  communication, annoyance):

        num_actors = self.sim_state.get_num_actors()
        cunits = self.sim_state.get_cunits()
        num_cus = len(cunits)

        actor_matrix = []
        for i in range(num_actors):
            matrix_row = []
            for j in range(num_actors):
                comm = math.ceil(communication[j][i])
                anno = math.ceil(annoyance[j][i])
                matrix_row.append((anno, comm))
            actor_matrix.append(matrix_row)

        for i in range(num_actors):
            print(str(actor_matrix[i]).replace('(','{').replace('[','{').replace(')','}').replace(']','}'))    

        task_forecast = self.sim_state._executed_events_per_actor[:]
        self.sim_state._executed_events_per_actor = [0]*num_actors
        print("task_forecast", task_forecast)
        total_load    = sum(task_forecast)




        capacity = []
        #non_zero_cap, non_zero_cu = float('inf'), None
        for k in self.sim_state._cu_units_data:
            if self.sim_state._cu_units_data[k]["executed"] != 0:
                non_zero_cap = self.sim_state._cu_units_data[k]["executed"]
                non_zero_cu  = k
                capacity += [int(self.sim_state._cu_units_data[k]["executed"]*1.01)] 
            else:
                capacity += [0]
            self.sim_state._cu_units_data[k]["executed"] = 0
        baseline_capacity = non_zero_cap/get_relative_speed(non_zero_cu)
        for i in range(num_cus):
            if capacity[i] == 0:
                capacity[i] = int(baseline_capacity*get_relative_speed(k))


        total_capacity    = sum(capacity)
        #for i in range(len(capacity)): capacity[i] = float(capacity[i])/total_capacity
        #for i in range(len(capacity)): capacity[i] = capacity[i]
        print("capacity", capacity)
        print("task_forecast", task_forecast)


        ddm_optimize(num_actors, ddm_prepare_actor_matrix(actor_matrix), task_forecast, num_cus, capacity)

    def delayed_on_window(self):
        binding = ddm_poll()
        if binding is not None:
            binding = convert_ddm_assignment_to_sim_assingment(binding)
        return binding
