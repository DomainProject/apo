from abc import ABC, abstractmethod

from metasimulation.window_operations.abstract_operations import WindowOperations
from metasimulation.SimulationModel.event_handlers import EVT


class BaseOperations(WindowOperations):

    def __init__(self):
        pass

    def on_window(self, cu_units_data, wct_ts, ending_simulation, min_vt, committed, time_window_size,
                  communication, annoyance):
       
        print("GVT", wct_ts, "SVT", min_vt, "COM", committed, "TH (com per msec)", committed / time_window_size)
       
        #print("comm")
        #for i in range(len(communication)):
        #    print(communication[i])
        #print("anno")
        #for i in range(len(annoyance)):
        #    print(annoyance[i])
       
        return min_vt

    def delayed_on_window(self):
        """Called multiple times after on_window, until a new binding is available. Return None if the binding is not"""
        """available yet, otherwise return the binding"""

        return None
