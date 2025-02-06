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
                comm = math.ceil(communication[j][i] / wct_ts)
                anno = math.ceil(annoyance[j][i] / wct_ts)
                matrix_row.append((anno, comm))
            actor_matrix.append(matrix_row)

        task_forecast = self.sim_state._executed_events_per_actor[:]
        total_load    = sum(task_forecast)
        for i in range(len(task_forecast)): task_forecast[i] = float(task_forecast[i])/total_load
        for i in range(len(task_forecast)): task_forecast[i] = task_forecast[i]*100
        for i in range(len(task_forecast)): task_forecast[i] = math.ceil(task_forecast[i])

        capacity = []
        non_zero_cap, non_zero_cu = float('inf'), None
        for k in self.sim_state._cu_units_data:
            if self.sim_state._cu_units_data[k]["executed"] != 0:
                non_zero_cap = self.sim_state._cu_units_data[k]["executed"]
                non_zero_cu  = k 
            self.sim_state._cu_units_data[k]["executed"] = 0
        baseline_capacity = non_zero_cap/get_relative_speed(non_zero_cu)
        for k in self.sim_state._cu_units_data:
            capacity += [int(baseline_capacity*get_relative_speed(k))]






        ddm_optimize(num_actors, ddm_prepare_actor_matrix(actor_matrix), task_forecast, num_cus, capacity)

    def delayed_on_window(self):
        binding = ddm_poll()
        if binding is not None:
            binding = convert_ddm_assignment_to_sim_assingment(binding)
        return binding
