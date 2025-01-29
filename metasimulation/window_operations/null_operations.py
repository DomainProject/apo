from metasimulation.window_operations.abstract_operations import WindowOperations


class NullOperations(WindowOperations):

    def __init__(self, sim_state):
        pass

    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        pass

    def delayed_on_window(self):
        return None
