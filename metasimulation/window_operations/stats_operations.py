from metasimulation.window_operations.abstract_operations import WindowOperations


class StatsOperations(WindowOperations):

    def __init__(self):
        pass

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        min_vt = super().on_window(cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                                   communication, annoyance)
        return min_vt

    def delayed_on_window(self, num_actors, current_assignment):
        return current_assignment
