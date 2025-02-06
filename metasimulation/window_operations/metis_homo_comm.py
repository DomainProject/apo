import math

from metasimulation.SimulationEngine.runtime_modules import global_constants_parameter_module as global_constants
from metasimulation.SimulationEngine.runtime_modules import hardware_parameter_module as hardware_constants

from metasimulation.SimulationEngine.sim import get_events_count_vector_in_next_window
from metasimulation.SimulationModel.hardware import build_cunits, get_capacity_vector, \
    convert_metis_assignment_to_sim_assingment,  get_communication_latency
from metasimulation.window_operations.abstract_operations import WindowOperations
from src.metis import ddmmetis_init, metis_get_partitioning, metis_baseline


class MetisHomogeneousCommunicationOperations(WindowOperations):

    def __init__(self, sim_state):
        print(f"initialize Metis...", end='')
        self.sim_state = sim_state
        ddmmetis_init(sim_state.get_num_actors())
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        if ending_simulation: return float('inf')
        min_vt = super().on_window(cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                                   communication, annoyance)
        num_actors = self.sim_state.get_num_actors()
        cunits = self.sim_state.get_cunits()
        num_cus = len(cunits)

        comm_matrix = []
        for i in range(num_actors):
            comm_row = []
            for j in range(num_actors):
                comm_row.append(math.ceil(communication[j][i] / wct_ts))
            comm_matrix.append(comm_row)

        capacity = [1.0/num_cus] * num_cus
        task_forecast = [1] * num_actors
        
        metis_baseline(num_actors, num_cus, comm_matrix, task_forecast, capacity)
        return min_vt

    def delayed_on_window(self):
        part = metis_get_partitioning()
        if not part:
            return None
        return convert_metis_assignment_to_sim_assingment(part)
