import ctypes



# Define METIS-compatible types
idx_t = ctypes.c_long  # Matches METIS's `idx_t` (long int)
real_t = ctypes.c_double  # Matches METIS's `real_t` (double)


last_ddm_total_actors_invocation = -1

_metisddm = ctypes.CDLL('./cmake-build-debug/src/libddmmetis.so')  ##if launch metis_test.py must be ../cmake-build-debug

_metisddm.ddmmetis_init.argtypes = [
    idx_t
]
_metisddm.ddmmetis_init.restype = None


import ctypes


def ddmmetis_init(total_actors):

    # Call the C function (assuming _metisddm.ddmmetis_init is defined elsewhere)
    _metisddm.ddmmetis_init(total_actors)




_metisddm.metis_heterogeneous_multilevel.argtypes = [
    idx_t,
    idx_t,
    ctypes.POINTER(idx_t),
    ctypes.POINTER(real_t),
    ctypes.POINTER(real_t),
    ctypes.POINTER(real_t),
    ctypes.POINTER(idx_t)
]
_metisddm.metis_heterogeneous_multilevel.restype = None

#ddm -> annoyance, communication cost, overload order: 0
#ddm_c1 -> overload, communication cost, annoyance order: 1
#ddm_c2 -> communication cost, overload,annoyance order: 2
#ddm_c3 -> overload+comm.cost, annoyance order: 3
#ddm_c4 -> overload (da ignorare) order: 4


#todo,: add parameter "flag" to choose among different metis implementations
def metis_heterogeneous_multilevel(total_actors, cus, tasks_forecast, capacity, comm_matrix, anno_matrix, msg_exch_cost):
    global last_ddm_total_actors_invocation
    if len(tasks_forecast) != total_actors:
        raise ValueError(f"tasks_forecast should have {total_actors} elements, but it has {len(tasks_forecast)}")

    arr_tasks = (idx_t * total_actors)(*map(lambda x: max(1, int(round(x))), tasks_forecast))
    

    min_capacity = min(capacity)
    max_capacity = max(capacity)
    if max_capacity != min_capacity:
        for i in range(cus):
            capacity[i] = (capacity[i] - min_capacity) / (max_capacity - min_capacity)
    else:
        for i in range(cus):
            capacity[i] = 1.0  # If all values are the same, set them to 1.0


    print(capacity)
    arr_capacity = (real_t * cus)(*capacity)

   
    flattened_comm_matrix = []
    for row in comm_matrix:
        flattened_comm_matrix.extend(row)

    flattened_anno_matrix = []
    for row in anno_matrix:
        flattened_anno_matrix.extend(row)

    flattened_msg_exch_cost_matrix = []
    for row in msg_exch_cost:
        flattened_msg_exch_cost_matrix.extend(row)

    arr_comm = (ctypes.c_double * (total_actors * total_actors))(*flattened_comm_matrix)
    arr_anno = (ctypes.c_double * (total_actors * total_actors))(*flattened_anno_matrix)
    arr_msg_exch = (ctypes.c_long * (cus * cus))(*flattened_msg_exch_cost_matrix)

    
    last_ddm_total_actors_invocation = total_actors

    _metisddm.metis_heterogeneous_multilevel(total_actors, cus, arr_tasks, arr_capacity, arr_comm, arr_anno, arr_msg_exch)



_metisddm.metis_communication.argtypes = [
    idx_t,
    idx_t,
    ctypes.POINTER(idx_t),
    ctypes.POINTER(real_t),
    ctypes.POINTER(idx_t),
]
_metisddm.metis_communication.restype = None

def metis_communication(total_actors, cus, tasks_forecast, comm_matrix, msg_exch_cost):
    global last_ddm_total_actors_invocation
    if len(tasks_forecast) != total_actors:
        raise ValueError(f"tasks_forecast should have {total_actors} elements, but it has {len(tasks_forecast)}")

    arr_tasks = (idx_t * total_actors)(*map(lambda x: max(1, int(round(x))), tasks_forecast))
    
    flattened_comm_matrix = []
    for row in comm_matrix:
        flattened_comm_matrix.extend(row)

 
    flattened_msg_exch_cost_matrix = []
    for row in msg_exch_cost:
        flattened_msg_exch_cost_matrix.extend(row)

    arr_comm = (ctypes.c_double * (total_actors * total_actors))(*flattened_comm_matrix)
    arr_msg_exch = (ctypes.c_long * (cus * cus))(*flattened_msg_exch_cost_matrix)

    
    last_ddm_total_actors_invocation = total_actors
    _metisddm.metis_communication(total_actors, cus, arr_tasks, arr_comm, arr_msg_exch)



_metisddm.metis_baseline.argtypes = [
    idx_t,
    idx_t,
    ctypes.POINTER(idx_t),
    ctypes.POINTER(real_t),
    ctypes.POINTER(real_t)
]
_metisddm.metis_baseline.restype = None

def metis_baseline(total_actors, cus, comm_matrix, tasks_forecast, capacity):
    global last_ddm_total_actors_invocation
    if len(tasks_forecast) != total_actors:
        raise ValueError(f"tasks_forecast should have {total_actors} elements, but it has {len(tasks_forecast)}")

    arr_tasks = (idx_t * total_actors)(*map(lambda x: max(1, int(round(x))), tasks_forecast))
    arr_capacity = (ctypes.c_double * cus)(*capacity)

    flattened_comm_matrix = []
    for row in comm_matrix:
        flattened_comm_matrix.extend(row)


    arr_comm = (ctypes.c_double * (total_actors * total_actors))(*flattened_comm_matrix)


    last_ddm_total_actors_invocation = total_actors
    _metisddm.metis_baseline(total_actors, cus,  arr_tasks, arr_comm, arr_capacity)



_metisddm.metis_get_partitioning.argtypes = []
_metisddm.metis_get_partitioning.restype = ctypes.POINTER(idx_t)


def metis_get_partitioning():
    global last_ddm_total_actors_invocation
    part = _metisddm.metis_get_partitioning()
    if not part:
        return None
    part = [part[i] for i in range(last_ddm_total_actors_invocation)]


    return part
