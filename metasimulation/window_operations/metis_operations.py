import random
import asyncio
import math

from metasimulation.SimulationModel.hardware import build_cunits
from metasimulation.window_operations.abstract_operations import WindowOperations
from src.metis import ddmmetis_init, metis_partitioning, metis_get_partitioning


class MetisOperations(WindowOperations):

    def __init__(self, sim_state):
        print(f"initialize Metis...", end='')
        self.sim_state = sim_state
        ddmmetis_init(num_actors, cus, comm_matrix, anno_matrix)
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        if ending_simulation: return float('inf')
        min_vt = super().on_window(cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                                   communication, annoyance)

        cus = len(build_cunits())
        capacity = get_capacity_vector()
        task_forecast = get_events_count_vector_in_next_window(wct_ts + time_window_size, num_actors)

        comm_matrix = []
        anno_matrix = []

        for i in range(num_actors):
            comm_row = []
            anno_row = []
            for j in range(num_actors):
                comm_row.append(math.ceil(communication[j][i] / min_vt))
                anno_row.append(math.ceil(annoyance[j][i] / min_vt))
            comm_matrix.append(comm_row)
            anno_matrix.append(anno_row)


        await self.async_metis_partitioning(num_actors, cus, task_forecast, capacity, comm_matrix, anno_matrix)
        return min_vt

    def delayed_on_window(self):
    async def delayed_on_window(self):
        #TODO call method to retrieve partitioning and try to install it
        # if no partitioning has been found return None
        cunits = build_cunits()
        actors = self.sim_state.get_num_actors()
        part = await asyncio.to_thread(metis_get_partitioning)
        if not part:
            return None
        return convert_metis_assignment_to_sim_assingment(part)
