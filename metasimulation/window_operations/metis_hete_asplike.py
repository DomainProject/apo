import math

from metasimulation.SimulationEngine.runtime_modules import hardware_parameter_module as hardware_constants
from metasimulation.SimulationEngine.sim import get_events_count_vector_in_next_window
from metasimulation.SimulationModel.hardware import convert_metis_assignment_to_sim_assingment, \
    get_communication_latency, get_relative_speed
from metasimulation.window_operations.abstract_operations import WindowOperations
from src.metis import ddmmetis_init, metis_get_partitioning, metis_heterogeneous_multilevel


class MetisHeterogeneousOperations(WindowOperations):

    def __init__(self, sim_state):
        print(f"initialize Metis...", end='')
        self.sim_state = sim_state
        # TODO: comm_matrix and anno_matrix must be taken in on_window
        ddmmetis_init(sim_state.get_num_actors())
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        if ending_simulation: return float('inf')
        min_vt = super().on_window(cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                                   communication, annoyance)
        num_actors = self.sim_state.get_num_actors()
        cunits = self.sim_state.get_cunits()
        cus = len(cunits)
        task_forecast = get_events_count_vector_in_next_window(wct_ts + time_window_size, num_actors)
        msg_exch_cost = [ [ int(get_communication_latency(x,y)/hardware_constants.comm_unitary_cost) for y in cunits] for x in cunits]
        comm_matrix = []
        anno_matrix = []
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


        for i in range(num_actors):
            comm_row = []
            anno_row = []
            for j in range(num_actors):
                comm_row.append(math.ceil(communication[j][i] / wct_ts))
                anno_row.append(math.ceil(annoyance[j][i] / wct_ts))
            comm_matrix.append(comm_row)
            anno_matrix.append(anno_row)

        task_forecast = [1] * num_actors

        if all(all(cell == 0 for cell in row) for row in anno_matrix):  # Check if all elements are 0
            anno_matrix = [[1] * len(anno_matrix[0]) for _ in range(len(anno_matrix))]


        #print("capacity : ", capacity)
        #print("msg exchange cost: ", msg_exch_cost)
        #print("task forecast: ",task_forecast)
        #print("annoyance matrix: ", anno_matrix)
        #print("communication matrix: ",comm_matrix)

        metis_heterogeneous_multilevel(num_actors, cus, task_forecast, capacity, comm_matrix, anno_matrix, msg_exch_cost)
        return min_vt

    def delayed_on_window(self):
        # TODO call method to retrieve partitioning and try to install it
        # if no partitioning has been found return None
        cunits = self.sim_state.get_cunits()
        actors = self.sim_state.get_num_actors()
        part = metis_get_partitioning()
        if not part:
            return None
        return convert_metis_assignment_to_sim_assingment(part)
