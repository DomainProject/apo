import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../')))



from metasimulation.hardware import build_cunits, convert_metis_assignment_to_sim_assingment, get_capacity_vector, communication_costs
from metasimulation.application import num_actors, comm_matrix, anno_matrix, task_unit_costs
from src.metis import ddmmetis_init, metis_partitioning, metis_get_partitioning




def test_metis():

    total_cus = len(build_cunits())
    total_actors = num_actors
    capacity = get_capacity_vector()
    
    cunits = build_cunits()
    
    print("Calling ddmmetis_init with parameters:", total_actors, total_cus)


    ddmmetis_init(total_actors, total_cus, comm_matrix, anno_matrix)


    tasks_forecast = [50, 50, 50, 50, 20, 50, 50, 80, 10, 10]

    metis_partitioning(total_actors, total_cus, tasks_forecast, capacity)
    
    part = metis_get_partitioning()

    print("This is the partition: " + str(part))

    assign = convert_metis_assignment_to_sim_assingment(part)

    print("This is the assignment: " + str(assign))


if __name__ == "__main__":
    test_metis()