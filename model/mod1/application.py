num_actors = 10 # number of actors in the system 

task_unit_costs  = [0.010]*num_actors # all actors have same expected cost per task in milliseconds!!!!!
task_anno_costs  = [0.050]*num_actors # all actors have same expected cost per task
comm_unitary_cost=  0.020   # MILLISECONDS to send a task with the faster communication channel

comm_matrix = [ [0]*num_actors ]*num_actors
anno_matrix = [ [0]*num_actors ]*num_actors

# all the following values are in task units per millisecond
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
    comm_matrix[i][j] = 1
    if i==j: 
      comm_matrix[i][j] = 10
      anno_matrix[i][j] = 0
