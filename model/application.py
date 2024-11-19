num_actors = 10 # number of actors in the system 
task_cost  = 15 # cost of a task unit (e.g. us)
anno_cost  = 30 # cost of a annoying unit (e.g. us)
base_load  = 10 # task units per seconds

workload   = [base_load]*num_actors # all actors have same expected cost per task

comm_matrix = [ [0]*num_actors ]*num_actors
anno_matrix = [ [0]*num_actors ]*num_actors

# all the following values are in task units per second
# the idea is that in an observation period:
# * the actor I sent comm_matrix[I][J] task request per second to the the actor J
# * the actor I sent anno_matrix[I][J] annoying task request per second to the actor J (this should be lower than comm_matrix[I][J])

comm_matrix[0] = [10, 6, 5, 4, 3, 2, 1, 0, 0, 0]
comm_matrix[1] = [6 ,10, 6, 5, 4, 3, 2, 1, 0, 0]
comm_matrix[2] = [5, 6 ,10, 6, 5, 4, 3, 2, 1, 0]
comm_matrix[3] = [4, 5, 6 ,10, 6, 5, 4, 3, 2, 1]
comm_matrix[4] = [3, 4, 5, 6 ,10, 6, 5, 4, 3, 2]
comm_matrix[5] = [2, 3, 4, 5, 6 ,10, 6, 5, 4, 3]
comm_matrix[6] = [1, 2, 3, 4, 5, 6 ,10, 6, 5, 4]
comm_matrix[7] = [0, 1, 2, 3, 4, 5, 6 ,10, 6, 5]
comm_matrix[8] = [0, 0, 1, 2, 3, 4, 5, 6 ,10, 6]
comm_matrix[9] = [0, 0, 0, 1, 2, 3, 4, 5, 6 ,10]

anno_matrix[0] = [0, 3, 2, 1, 0, 0, 0, 0, 0, 0]
anno_matrix[1] = [3 ,0, 3, 2, 1, 0, 0, 0, 0, 0]
anno_matrix[2] = [2, 3 ,0, 3, 2, 1, 0, 0, 0, 0]
anno_matrix[3] = [1, 2, 3 ,0, 3, 2, 1, 0, 0, 0]
anno_matrix[4] = [0, 1, 2, 3 ,0, 3, 2, 1, 0, 0]
anno_matrix[5] = [0, 0, 1, 2, 3 ,0, 3, 2, 1, 0]
anno_matrix[6] = [0, 0, 0, 1, 2, 3 ,0, 3, 2, 1]
anno_matrix[7] = [0, 0, 0, 0, 1, 2, 3 ,0, 3, 2]
anno_matrix[8] = [0, 0, 0, 0, 0, 1, 2, 3 ,0, 3]
anno_matrix[9] = [0, 0, 0, 0, 0, 0, 1, 2, 3, 0]




for i in range(num_actors):
  for j in range(num_actors):
    anno_matrix[i][j] = 0
    comm_matrix[i][j] = 0
    if i==j: comm_matrix[i][j] = base_load

