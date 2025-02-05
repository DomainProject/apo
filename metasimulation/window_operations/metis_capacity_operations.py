import math

from metasimulation.SimulationEngine.runtime_modules import global_constants_parameter_module as global_constants
from metasimulation.SimulationEngine.runtime_modules import hardware_parameter_module as hardware_constants

from metasimulation.SimulationEngine.sim import get_events_count_vector_in_next_window
from metasimulation.SimulationModel.hardware import build_cunits, get_capacity_vector, \
    convert_metis_assignment_to_sim_assingment,  get_communication_latency
from metasimulation.window_operations.abstract_operations import WindowOperations
from src.metis import ddmmetis_init, metis_get_partitioning, metis_capacity


class MetisCapacityOperations(WindowOperations):

    def __init__(self, sim_state):
        print(f"initialize Metis...", end='')
        self.sim_state = sim_state
        ddmmetis_init(sim_state.get_num_actors(), len(sim_state.get_cunits()))
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        if ending_simulation: return float('inf')
        min_vt = super().on_window(cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                                   communication, annoyance)
        num_actors = self.sim_state.get_num_actors()
        cunits = self.sim_state.get_cunits()
        cus = len(cunits)
        capacity = [1] * cus
        task_forecast = get_events_count_vector_in_next_window(wct_ts + time_window_size, num_actors)
        comm_matrix = []

        for i in range(num_actors):
            comm_row = []
            for j in range(num_actors):
                comm_row.append(0)
            comm_matrix.append(comm_row)


        for i in range(len(task_forecast)): task_forecast[i] += 1

        augmented_vertexes = num_actors*cus 

        forecast_capacity_metric = []
        for i in range(num_actors):
            for j in range(cus):
                forecast_capacity_metric.append(math.ceil(task_forecast[i]/capacity[j]))

        print("capacity : ", capacity)
        print("task forecast: ", task_forecast)
        print("forecast_capacity_metric: ", forecast_capacity_metric)
        metis_capacity(num_actors, cus, comm_matrix, forecast_capacity_metric)
        return min_vt

    def delayed_on_window(self):
        # TODO call method to retrieve partitioning and try to install it
        # if no partitioning has been found return None
        cunits = self.sim_state.get_cunits()
        actors = self.sim_state.get_num_actors()
        speed = []
        part = metis_get_partitioning()
        if not part:
            return None
        return convert_metis_assignment_to_sim_assingment(part)
