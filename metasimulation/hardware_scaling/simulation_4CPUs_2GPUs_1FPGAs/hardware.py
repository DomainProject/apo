cu_types = {
'cpu' : {
    'num_units':4,
    'relative_speed':1.0,
    },
'gpu' : {'num_units':2, 'relative_speed':4},
'fpga': {'num_units':1, 'relative_speed':2},
}

comm_unitary_cost =  0.020

communication_costs = {}
for k1 in cu_types:
  communication_costs[k1] = {}
  for k2 in cu_types:
    communication_costs[k1][k2] = 1
    if k1 == 'gpu'  and k2 == 'cpu': communication_costs[k1][k2] = 5
    if k2 == 'gpu'  and k1 == 'cpu': communication_costs[k1][k2] = 5
    if k1 == 'fpga' and k2 == 'cpu': communication_costs[k1][k2] = 10
    if k2 == 'fpga' and k1 == 'cpu': communication_costs[k1][k2] = 10
    if k1 == 'fpga' and k2 == 'gpu': communication_costs[k1][k2] = 20
    if k2 == 'fpga' and k1 == 'gpu': communication_costs[k1][k2] = 20
