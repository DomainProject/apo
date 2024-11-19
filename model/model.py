from application import *
from platform import *

def assignment1(num_actors,cus):
  assignment = []
  while(len(assignment) < num_actors):
    for cu in cus:
        assignment += [cu]
  return sorted(assignment[:num_actors])

def assignment2(num_actors,cus):
  return ['cpu_0', 'cpu_0', 'cpu_0', 'cpu_0', 'cpu_1', 'cpu_1', 'cpu_1', 'cpu_1', 'gpu_0', 'fpga_0']

def assignment3(num_actors,cus):
  return ['cpu_0', 'cpu_0', 'cpu_0', 'cpu_0', 'cpu_1', 'cpu_1', 'cpu_1', 'cpu_1', 'gpu_0', 'fpga_0']

def assignment4(num_actors,cus):
  return ['fpga_0', 'fpga_0', 'fpga_0', 'fpga_0', 'fpga_0', 'fpga_0', 'fpga_0', 'gpu_0', 'cpu_0', 'cpu_1']

def all_cpu(num_actors,cus):
  return ['cpu_0']*num_actors

def all_gpu(num_actors,cus):
  return ['gpu_0']*num_actors

def all_fpga(num_actors,cus):
  return ['fpga_0']*num_actors


def compute_total_work(assignment):
  num_actors = len(assignment)
  total_work = workload[:]    #total (own + received) units per sec
  total_anno = [0]*num_actors #total annoying (subset of received) units per sec

  for i in range(num_actors):
    for j in range(num_actors):
      total_work[i] += comm_matrix[j][i]
      if not on_same_unit(assignment, i,j):
          total_anno[i] += anno_matrix[j][i]
  return total_work, total_anno


def compute_annoyance_cost(total_work, total_anno):
  num_actors = len(total_work)
  eff_anno = [anno_cost]*num_actors #latency in us
  for i in range(num_actors):
    eff_anno[i] *= float(total_anno[i])/total_work[i] #expected annoyance cost, aka (annoyance cost)*(probability of being annoyed)
  return eff_anno

def compute_communication_cost(assignment, total_work):
  num_actors = len(assignment)
  fres = []
  for i in range(num_actors):
    tmp = {}                        # communication rate for actor I towards a specific device
    for k in cu_types: tmp[k] = 0       

    tot = 0                         # total output communication rate

    my_device = get_dev_from_cu(assignment[i])
    
    for j in range(num_actors):
      tot    += comm_matrix[i][j]       
      if on_same_unit(assignment, i,j): continue #no cost when communicating to the same  computing unit

      k = get_dev_from_cu(assignment[j])
      tmp[k] += comm_matrix[i][j]

    for k in tmp: float(tmp[k]) / tot #probability to communicate with a specific device

    for k in tmp: float(tmp[k]) * communication_costs[my_device][k] #weighted cost for communicating with a specific device
    res = 0
    for k in tmp: res+=float(tmp[k]) #expected communication cost per individual communication
    fres +=[res*tot/total_work[i]] #total expected communication 
  return fres


def assignment_renaming(assignment):
  new_assignment = []
  translation = {}
  used_labels = set([])
  
  for i in assignment:
    if i not in translation:
      dev = i.split('_')[0]
      count = 0
      while True:
        new_label = f"{dev}_{count}"
        if new_label not in used_labels:
          used_labels.add(new_label)
          translation[i] = new_label
          break
        count += 1
    new_assignment += [translation[i]]
  return new_assignment

def compute_whole_model(cu_units, assignment, log=False):
  if log: print(f"assignment:{assignment}")
  total_work,total_anno = compute_total_work(assignment)
  if log: print(f"eff_load:{[task_cost]*num_actors}")
  eff_anno = compute_annoyance_cost(total_work, total_anno)
  if log: print(f"eff_anno:{eff_anno}")
  eff_comm = compute_communication_cost(assignment, total_work)
  if log: print(f"eff_comm:{eff_comm}")

  latencies_per_actor  = [0]*num_actors
  for i in range(num_actors):
    latencies_per_actor[i] += eff_comm[i] + eff_anno[i] + task_cost

  latencies_per_cu = {}
  for i in range(num_actors):
      cu = assignment[i]
      if cu not in latencies_per_cu: latencies_per_cu[cu] = {'sum':0, 'load':0}
      latencies_per_cu[cu]['sum']  += latencies_per_actor[i]*total_work[i]    
      latencies_per_cu[cu]['load'] += total_work[i]

  for cu in latencies_per_cu:
    latencies_per_cu[cu]['lat'] = latencies_per_cu[cu]['sum']/latencies_per_cu[cu]['load']

  for cu in latencies_per_cu:
    dev = cu.split('_')[0]
    latencies_per_cu[cu]['lat'] /= cu_types[dev]['relative_speed']
    if latencies_per_cu[cu]['load'] > cu_types[dev]['capacity_cu']:
        latencies_per_cu[cu]['lat'] *= cu_types[dev]['overload_penalty'] * (latencies_per_cu[cu]['load'] - cu_types[dev]['capacity_cu'])

  tot = 0
  for cu in latencies_per_cu:
    tot += 1000.0*1000.0/latencies_per_cu[cu]['lat']
    print(f"{cu}\t:{latencies_per_cu[cu]['lat']} us - {1000.0*1000.0/latencies_per_cu[cu]['lat']} task per sec - {latencies_per_cu[cu]['load']}")
    #print(f"{cu}:\t{1000.0*1000.0/latencies_per_cu[cu]['lat']} task per sec")

  if log: print(f"total th: {tot} task per sec")    
  if log: print()

  return tot

cu_units = build_cunits()
global_solutions = {}

print(f"cu_units:{cu_units}")


res_sol = []
res_thr = 0

import itertools

progress = 0
end_cnts = len(cu_units)**num_actors
modulo = int(end_cnts*0.05)
print(progress,end_cnts,modulo)



for assignment in itertools.product(cu_units, repeat=num_actors):
  break
  assignment = list(assignment)
  assignment = assignment_renaming(assignment)

  t = tuple(assignment)
  if t not in global_solutions: 
    tot = compute_whole_model(cu_units, assignment)
    if tot > res_thr:
      res_thr = tot
      res_sol = [assignment[:]]
    if tot == res_thr:
      res_sol += [assignment[:]]

  progress+=1
  if (progress % modulo) == 0: print(progress/end_cnts)

  global_solutions[t] = tot

for sol in res_sol:
    print(sol)
print(res_thr)


for f in [all_cpu, all_gpu, all_fpga, assignment1, assignment2, assignment3, assignment4]:
    assignment = f(num_actors, cu_units)
    tot = compute_whole_model(cu_units, assignment, True)
