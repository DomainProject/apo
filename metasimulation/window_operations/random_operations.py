import random

from metasimulation.SimulationModel.hardware import build_cunits
from metasimulation.window_operations.abstract_operations import WindowOperations


class RandomOperations(WindowOperations):

    def __init__(self, sim_state):
        print(f"initialize Random...", end='')
        self.sim_state = sim_state
        random.seed(a=879156, version=2)
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        pass

    def delayed_on_window(self):
        cunits = build_cunits()
        actors = self.sim_state.get_num_actors()
        return [random.choice(cunits) for _ in range(actors)]
