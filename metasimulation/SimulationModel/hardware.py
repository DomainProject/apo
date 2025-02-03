from metasimulation.SimulationParameters.hardware import *


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
def on_same_device(assignment, i, j):
    return get_dev_from_cu(assignment[i]) == get_dev_from_cu(assignment[j])


# tells if two actors are on the same computing unit
def on_same_unit(assignment, i, j):
    return assignment[i] == assignment[j]


# get latency for communicating between two devices
def get_communication_latency(cu_a, cu_b):
    if cu_a == cu_b:
        return comm_unitary_cost / 2
    return communication_costs[get_dev_from_cu(cu_a)][get_dev_from_cu(cu_b)] * comm_unitary_cost


def get_relative_speed(cu_a):
    return cu_types[get_dev_from_cu(cu_a)]['relative_speed']


def get_capacity_vector():
    res = []
    for k in cu_types:
        res += [cu_types[k]['capacity_cu']] * cu_types[k]['num_units']
    return res


def convert_ddm_assignment_to_sim_assingment(ddm_assignment):
    # ddm_assignment is a list of integers, where each integer represents the computing unit index
    # we need to convert it to a list of strings, where each string represents the computing unit label
    cunits = build_cunits()
    return [cunits[i] for i in ddm_assignment]


def convert_metis_assignment_to_sim_assingment(partition):
    assign = []
    cunits = build_cunits()

    count_dict = {}
    for index in partition:
        if index in count_dict:
            count_dict[index] += 1
        else:
            count_dict[index] = 1

    #print(partition, speed)
    sorted_indices = sorted(count_dict.keys(), key=lambda x: count_dict[x], reverse=True)


    unit_mapping = {index: cunits[i] for i, index in enumerate(sorted_indices)}
    assign = [unit_mapping[i] for i in partition]



    #print(partition)
    # round robin assignement
    # assign = [cunits[i] for i in partition]
    return assign

def convert_metis_assignment_to_sim_assingment_enhanced(partition, speed):
    assign = []
    cunits = build_cunits()

    count_dict = {}
    for index in partition:
        if index in count_dict:
            count_dict[index] += 1
        else:
            count_dict[index] = 1

    #print(partition, speed)
    sorted_indices = sorted(count_dict.keys(), key=lambda x: count_dict[x], reverse=True)

    sorted_cunits = sorted(enumerate(speed), key=lambda x: x[1], reverse=True)

    # Ensure we have enough computing units for the partitions
    unit_mapping = {}
    num_cunits = len(sorted_cunits)
    num_partitions = len(sorted_indices)

    # Limit the mapping to the number of available computing units
    for i in range(min(num_cunits, num_partitions)):
        unit_idx = sorted_cunits[i][0]  # The index of the CU in the sorted list
        unit_mapping[sorted_indices[i]] = cunits[unit_idx]

    #unit_mapping = {index: cunits[i] for i, index in enumerate(sorted_indices)}
    assign = [unit_mapping[i] for i in partition]



    #print(partition)
    # round robin assignement
    # assign = [cunits[i] for i in partition]
    return assign