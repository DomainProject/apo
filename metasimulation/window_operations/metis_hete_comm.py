import math

from metasimulation.SimulationModel.hardware import convert_metis_assignment_to_sim_assingment, get_relative_speed
from metasimulation.window_operations.abstract_operations import WindowOperations
from src.metis import ddmmetis_init, metis_get_partitioning, metis_baseline





class MetisCommunicationOperations(WindowOperations):

    def __init__(self, sim_state):
        print(f"initialize Metis...", end='')
        self.sim_state = sim_state
        ddmmetis_init(sim_state.get_num_actors())




        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):

        num_actors = self.sim_state.get_num_actors()
        cunits = self.sim_state.get_cunits()
        num_cus = len(cunits)

        comm_matrix = []
        for i in range(num_actors):
            comm_row = []
            for j in range(num_actors):
                comm_row.append(math.ceil(communication[j][i] / wct_ts))
            comm_matrix.append(comm_row)



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
            capacity += [baseline_capacity*get_relative_speed(k)]


        total_capacity    = sum(capacity)
        for i in range(len(capacity)): capacity[i] = float(capacity[i])/total_capacity
        for i in range(len(capacity)): capacity[i] = capacity[i]

        metis_baseline(num_actors, num_cus, comm_matrix, task_forecast, capacity)

    def delayed_on_window(self):
        part = metis_get_partitioning()
        if not part:
            return None
        return convert_metis_assignment_to_sim_assingment(part)

