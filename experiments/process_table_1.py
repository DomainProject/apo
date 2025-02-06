import os
from os import listdir
from os.path import isfile, join


sims = [1,2,3]
p = "res_path"
res = {}
for i in sims:
  sim = f"sim_{i}"
  res[sim]={}
  path=f"{p}/{sim}"
  onlyfiles = [f"{f}" for f in listdir(path) if isfile(join(path, f))]
  for f in onlyfiles:
    solver = '-'.join(f.split('-')[:-1])
    file=open(path+'/'+f)
    l1 = None
    l2 = None
    for line in file.readlines():
      l1 = l2
      l2 = line.strip()
    file.close()
    l1 = l1.split(' ')
    l1 = [float(l1[-4]), float(l1[-1])]
    l1 = [min(l1), l1[-1]]
    if solver not in res[sim]: res[sim][solver] = []
    res[sim][solver] += [l1]

for i in res:
  print(i)
  for j in ["random", "metis-homo-node", "metis-homo-comm", "ddm", "ddm_c1", "ddm_c2", "ddm_c3", "ddm_c4"]:
   avg = [ k[0]/k[1] for k in res[i][j] ]
   avg = sum(avg)/len(avg)
   print(f"|-{j}:{avg}")
