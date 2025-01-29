from metasimulation.window_operations.abstract_operations import WindowOperations


class StatsOperations(WindowOperations):

    def __init__(self, sim_state):
        print(f"initialize Stats...", end='')
        self.sim_state = sim_state
        print(f"done")

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        min_vt = super().on_window(cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                                   communication, annoyance)
        return min_vt

    def delayed_on_window(self):
        return self.sim_state.get_assignment()
