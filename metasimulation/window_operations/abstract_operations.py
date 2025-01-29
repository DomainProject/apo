from abc import ABC, abstractmethod

class WindowOperations(ABC):

    @abstractmethod
    def __init__(self, sim_state):
        pass

    @abstractmethod
    def on_window(self, cu_units_data, wct_ts, ending_simulation, min_vt, committed, time_window_size,
                  communication, annoyance):
        pass

    @abstractmethod
    def delayed_on_window(self):
        """Called multiple times after on_window, until a new binding is available. Return None if the binding is not"""
        """available yet, otherwise return the binding"""
        return None
