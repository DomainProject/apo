import random

from metasimulation.hardware import build_cunits
from metasimulation.window_operations.abstract_operations import WindowOperations


class RandomOperations(WindowOperations):

    def __init__(self):
        pass

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        if ending_simulation: return float('inf')
        min_vt = super().on_window(cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                                   communication, annoyance)
        return min_vt

    def delayed_on_window(self, num_actors, current_assignment):
        cunits = build_cunits()
        actors = num_actors
        return [random.choice(cunits) for _ in range(actors)]
