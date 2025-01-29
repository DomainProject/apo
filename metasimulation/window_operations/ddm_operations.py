import math

from metasimulation.Simulator.sim import get_events_count_vector_in_next_window
from metasimulation.application import num_actors
from metasimulation.hardware import build_cunits, communication_costs, get_capacity_vector, \
    convert_ddm_assignment_to_sim_assingment
from metasimulation.window_operations.abstract_operations import WindowOperations
from src.ddm import ddm_init, ddm_optimize, ddm_prepare_actor_matrix, ddm_poll


class DdmOperations(WindowOperations):

    def __init__(self):
        print(f"initialize DDM...", end='')
        cunits = build_cunits()
        cus = [{'cpu': 1, 'gpu': 2, 'fpga': 4}[x.split('_')[0]] for x in cunits]
        total_cus = len(cus)
        msg_exch_cost = [[communication_costs[x.split('_')[0]][y.split('_')[0]] for y in cunits] for x in cunits]
        runnable_on = [7] * num_actors
        ddm_init(total_cus, num_actors, cus, msg_exch_cost, runnable_on)
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        if ending_simulation: return float('inf')
        min_vt = super().on_window(cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                                   communication, annoyance)

        actor_matrix = []
        for i in range(num_actors):
            matrix_row = []
            for j in range(num_actors):
                comm = math.ceil(communication[j][i] / min_vt)
                anno = math.ceil(annoyance[j][i] / min_vt)
                matrix_row.append((anno, comm))
            actor_matrix.append(matrix_row)

        #print(actor_matrix)
        ddm_optimize(num_actors, ddm_prepare_actor_matrix(actor_matrix),
                     get_events_count_vector_in_next_window(wct_ts + time_window_size, num_actors), len(build_cunits()),
                     get_capacity_vector())

        return min_vt

    def delayed_on_window(self, num_actors, current_assignment):
        binding = ddm_poll()
        if binding is not None:
            binding = convert_ddm_assignment_to_sim_assingment(binding)
        return binding
