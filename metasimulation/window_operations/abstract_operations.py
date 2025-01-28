from abc import ABC, abstractmethod

from metasimulation.SimulationEngine.sim import is_there_any_pending_evt, schedule_event
from metasimulation.SimulationModel.event_handlers import EVT


class WindowOperations(ABC):

    @abstractmethod
    def __init__(self):
        pass

    @abstractmethod
    def on_window(self, cu_units_data, wct_ts, ending_simulation, min_vt, committed, time_window_size,
                  communication, annoyance):
        return float('inf')
        
    @abstractmethod
    def delayed_on_window(self):
        """Called multiple times after on_window, until a new binding is available. Return None if the binding is not"""
        """available yet, otherwise return the binding"""
        return None
