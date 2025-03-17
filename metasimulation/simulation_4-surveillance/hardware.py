cu_types = {
'cpu' : {
    'num_units':2,            # numer of computing unit per this device
    'relative_speed':1.0,     # relative speed w.r.t. to slowest device (1.0 is the slowest)
    #'capacity_cu':4,          # per computing unit capacity in terms of task units before being overloaded
    #'overload_penalty':10    # slowing factor for running as overloaded
    },
'gpu' : {'num_units':1, 'relative_speed':4}, # , 'capacity_cu':2, 'overload_penalty': 30},
'fpga': {'num_units':1, 'relative_speed':2} # , 'capacity_cu':2, 'overload_penalty': 60},
}

comm_unitary_cost =  0.020   # MILLISECONDS to send a task with the faster communication channel
#all the following is commu_unitary_costs

communication_costs = {}
for k1 in cu_types:
  communication_costs[k1] = {}
  for k2 in cu_types:
    communication_costs[k1][k2] = 1                     # slowing factor for communication: xpu1->xpu1 fastest
    if k1 == 'gpu'  and k2 == 'cpu': communication_costs[k1][k2] = 5    #                                   cpu->xpu and xpu->cpu have communication cost multiplied by 2
    if k2 == 'gpu'  and k1 == 'cpu': communication_costs[k1][k2] = 5    #                                   cpu->xpu and xpu->cpu have communication cost multiplied by 2

    if k1 == 'fpga' and k2 == 'cpu': communication_costs[k1][k2] = 10    #                                   cpu->xpu and xpu->cpu have communication cost multiplied by 2
    if k2 == 'fpga' and k1 == 'cpu': communication_costs[k1][k2] = 10    #                                   cpu->xpu and xpu->cpu have communication cost multiplied by 2

    if k1 == 'fpga' and k2 == 'gpu': communication_costs[k1][k2] = 20    #                                   cpu->xpu and xpu->cpu have communication cost multiplied by 2
    if k2 == 'fpga' and k1 == 'gpu': communication_costs[k1][k2] = 20    #                                   cpu->xpu and xpu->cpu have communication cost multiplied by 2

    

# 4 cpu 1 1
# attori 100


# percentuale hot spot 10%
# ogni 10 invia evento foglia

