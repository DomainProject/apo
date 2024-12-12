cu_types = {
'cpu' : {
    'num_units':2,            # numer of computing unit per this device
    'relative_speed':1.0,     # relative speed w.r.t. to slowest device (1.0 is the slowest)
    'capacity_cu':4,          # per computing unit capacity in terms of task units before being overloaded
    'overload_penalty':10    # slowing factor for running as overloaded
    },
'gpu' : {'num_units':1, 'relative_speed':5 , 'capacity_cu':2, 'overload_penalty': 30},
'fpga': {'num_units':1, 'relative_speed':10, 'capacity_cu':1, 'overload_penalty': 60},
}

comm_unitary_cost=  0.020   # MILLISECONDS to send a task with the faster communication channel
#all the following is commu_unitary_costs

communication_costs = {}
for k1 in cu_types:
  communication_costs[k1] = {}
  for k2 in cu_types:
    communication_costs[k1][k2] = 1                     # slowing factor for communication: xpu1->xpu1 fastest
    if k1 != 'cpu': communication_costs[k1][k2] += 2    #                                   cpu->xpu and xpu->cpu have communication cost multiplied by 2
    if k2 != 'cpu': communication_costs[k1][k2] += 4    #                                   xpu1->xpu2  have communication cost multiplied by 4 when xpu1 and xpu2 are not cpus



# UTILITY FUNCTIONS

# creates an array of computing unit labels
def build_cunits():
  res = []
  for k in cu_types:
      res += [f"{k}_{v}" for v in range(cu_types[k]['num_units'])]
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
def get_communication_latency(dev_a, dev_b):
  if dev_a == dev_b: return 0
  return communication_costs[get_dev_from_cu(dev_a)][get_dev_from_cu(dev_b)]*comm_unitary_cost