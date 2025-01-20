from abc import ABC, abstractmethod

from metasimulation.Simulator.sim import is_there_any_pending_evt, schedule_event, EVT


class WindowOperations(ABC):

    @abstractmethod
    def __init__(self):
        pass

    @abstractmethod
    def on_window(self, cu_units_data, wct_ts, ending_simulation, traces, committed_idxs, time_window_size,
                  communication, annoyance):
        if ending_simulation: return float('inf')
        min_vt = float('inf')
        for k in cu_units_data:
            tmp = float('inf')
            if len(cu_units_data[k]['queue']) > 0:    tmp = cu_units_data[k]['queue'][0][0]
            min_vt = min(min_vt, tmp)

        committed = 0
        for k in traces:
            while committed_idxs[k] < len(traces[k]) and traces[k][committed_idxs[k]][0] < min_vt:
                committed_idxs[k] += 1
                committed += 1

        print("GVT", wct_ts, "SVT", min_vt, "COM", committed, "TH (com per msec)", committed / time_window_size)
        if is_there_any_pending_evt(): schedule_event(wct_ts + time_window_size, 0, EVT.TIME_WINDOW)
        return min_vt

    @abstractmethod
    def delayed_on_window(self, num_actors, current_assignment):
        """Called multiple times after on_window, until a new binding is available. Return None if the binding is not"""
        """available yet, otherwise return the binding"""

        return current_assignment
