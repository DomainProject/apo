import sys
import os

from metasimulation.SimulationModel.hardware import convert_metis_assignment_to_sim_assingment

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../')))

from src.metis import ddmmetis_init, metis_partitioning, metis_get_partitioning




def test_metis():

    total_cus = 4
    total_actors = 10
    cus = [1, 1, 2, 4]
    msg_exch_cost = [
        [1, 1, 5, 5],
        [1, 1, 5, 5],
        [3, 3, 7, 7],
        [3, 3, 7, 7]
    ]

    runnable_on = [7, 7, 7, 7, 7, 7, 7, 7, 7, 7]
    cu_capacity = [4, 4, 2, 1]

    actors = [[(1, 5), (1, 5), (0, 5), (1, 6), (1, 5), (1, 5), (1, 5), (1, 5), (1, 6), (1, 5)],
              [(1, 5), (1, 4), (1, 5), (1, 4), (1, 5), (1, 5), (1, 5), (1, 5), (1, 5), (1, 5)],
              [(1, 3), (1, 3), (1, 4), (1, 4), (1, 4), (1, 4), (1, 4), (1, 4), (1, 3), (1, 4)],
              [(1, 4), (1, 4), (1, 3), (1, 4), (1, 4), (1, 4), (1, 4), (1, 4), (1, 3), (1, 4)],
              [(1, 4), (1, 4), (1, 3), (1, 4), (1, 4), (1, 3), (1, 4), (1, 4), (1, 4), (1, 4)],
              [(1, 5), (1, 5), (1, 5), (1, 5), (0, 4), (1, 4), (1, 5), (1, 5), (1, 5), (1, 4)],
              [(1, 4), (1, 3), (1, 4), (1, 4), (1, 4), (1, 4), (1, 4), (1, 4), (1, 4), (1, 4)],
              [(1, 9), (1, 8), (1, 8), (1, 9), (1, 9), (1, 8), (1, 9), (1, 6), (1, 9), (1, 10)],
              [(1, 6), (1, 5), (1, 6), (1, 6), (1, 5), (1, 5), (1, 5), (1, 6), (1, 5), (1, 5)],
              [(1, 5), (1, 5), (1, 4), (1, 5), (1, 5), (1, 4), (1, 4), (1, 5), (1, 5), (1, 4)]]

    print("Calling ddmmetis_init with parameters:", total_actors, total_cus)

    ddmmetis_init(total_actors, total_cus)

    tasks_forecast = [50, 50, 50, 50, 20, 50, 50, 80, 10, 10]

    # first component of each actor is the annoyance
    anno_matrix = [[actor[0] for actor in row] for row in actors]
    # second component of each actor is the communication cost
    comm_matrix = [[actor[1] for actor in row] for row in actors]

    metis_partitioning(total_actors, total_cus, tasks_forecast, cu_capacity, comm_matrix, anno_matrix)
    metis_partitioning(total_actors, total_cus, tasks_forecast, cu_capacity, comm_matrix, anno_matrix, msg_exch_cost)

    part = metis_get_partitioning()

    print("This is the partition:", part)
    # expected = [3, 0, 3, 0, 3, 2, 3, 3, 3, 0]


    # assign = convert_metis_assignment_to_sim_assingment(part)
    #
    # print("This is the assignment: " + str(assign))


if __name__ == "__main__":
    test_metis()
