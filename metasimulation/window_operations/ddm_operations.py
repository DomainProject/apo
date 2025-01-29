import math

from metasimulation.SimulationEngine.sim import get_events_count_vector_in_next_window
from metasimulation.window_operations.abstract_operations import WindowOperations
from src.ddm import ddm_init, ddm_optimize, ddm_prepare_actor_matrix, ddm_poll
from metasimulation.SimulationModel.hardware import get_communication_latency, comm_unitary_cost, get_capacity_vector, \
       convert_ddm_assignment_to_sim_assingment


class DdmOperations(WindowOperations):

    def __init__(self, sim_state):
        print(f"initialize DDM...", end='')
        cunits = sim_state.get_cunits()
        cus = [{'cpu': 1, 'gpu': 2, 'fpga': 4}[x.split('_')[0]] for x in cunits]
        total_cus = len(cus)
        msg_exch_cost = [ [ int(get_communication_latency(x,y)/comm_unitary_cost) for y in cunits] for x in cunits]
        runnable_on = [7] * sim_state.get_num_actors()
        ddm_init(total_cus, sim_state.get_num_actors(), cus, msg_exch_cost, runnable_on)
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, min_vt, committed, time_window_size,
                  communication, annoyance):

        actor_matrix = []
        num_actors = len(communication)
        for i in range(num_actors):
            matrix_row = []
            for j in range(num_actors):
                comm = math.ceil(communication[j][i] / wct_ts)
                anno = math.ceil(annoyance[j][i] / wct_ts)
                matrix_row.append((anno, comm))
            actor_matrix.append(matrix_row)

        ddm_optimize(num_actors, ddm_prepare_actor_matrix(actor_matrix),
                     get_events_count_vector_in_next_window(wct_ts + time_window_size, num_actors), len(cu_units_data),
                     get_capacity_vector())
        

    def delayed_on_window(self):
        binding = ddm_poll()
        if binding is not None:
            binding = convert_ddm_assignment_to_sim_assingment(binding)
        return binding
