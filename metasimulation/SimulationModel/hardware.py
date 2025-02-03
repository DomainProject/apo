#from metasimulation.SimulationParameters.hardware import *
# hardware_parameter_module is a dynamically loaded module containing cu_types and communication matrix
# UTILITY FUNCTIONS


from metasimulation.SimulationEngine.runtime_modules import *

# creates an array of computing unit labels
def build_cunits():
  res = []
  for k in hardware_parameter_module.cu_types:
      res += [f"{k}_{v}" for v in range(hardware_parameter_module.cu_types[k]['num_units'])]
  return res

# get device type from computing unit
def get_dev_from_cu(cu_unit_label):
  return cu_unit_label.split('_')[0]


# tells if two actors are on the same device
def on_same_device(assignment,i,j):
  return get_dev_from_cu(assignment[i]) == get_dev_from_cu(assignment[j])

# tells if two actors are on the same computing unit
def on_same_unit(assignment,i,j):
  return assignment[i] == assignment[j]


# get latency for communicating between two devices
def get_communication_latency(cu_a, cu_b):
  if cu_a == cu_b: 
    return hardware_parameter_module.comm_unitary_cost/2
  return hardware_parameter_module.communication_costs[get_dev_from_cu(cu_a)][get_dev_from_cu(cu_b)]*hardware_parameter_module.comm_unitary_cost

def get_relative_speed(cu_a):
  return hardware_parameter_module.cu_types[get_dev_from_cu(cu_a)]['relative_speed']

def get_capacity_vector():
    res = []
    for k in hardware_parameter_module.cu_types:
        #res += [hardware_parameter_module.cu_types[k]['capacity_cu']]*hardware_parameter_module.cu_types[k]['num_units']
        res += [3]*hardware_parameter_module.cu_types[k]['num_units']
    return res

def convert_ddm_assignment_to_sim_assingment(ddm_assignment):
    # ddm_assignment is a list of integers, where each integer represents the computing unit index
    # we need to convert it to a list of strings, where each string represents the computing unit label
    cunits = build_cunits()
    return [cunits[i] for i in ddm_assignment]
