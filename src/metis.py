import ctypes



# Define METIS-compatible types
idx_t = ctypes.c_long  # Matches METIS's `idx_t` (long int)
real_t = ctypes.c_double  # Matches METIS's `real_t` (double)


last_ddm_total_actors_invocation = -1

_metisddm = ctypes.CDLL('./cmake-build-debug/src/libddmmetis.so')  ##if launch metis_test.py must be ../cmake-build-debug

_metisddm.ddmmetis_init.argtypes = [
    idx_t,
    idx_t

]
_metisddm.ddmmetis_init.restype = None


import ctypes


def ddmmetis_init(total_actors, cus):

    # Call the C function (assuming _metisddm.ddmmetis_init is defined elsewhere)
    _metisddm.ddmmetis_init(total_actors, cus)




_metisddm.metis_partitioning.argtypes = [
    idx_t,
    idx_t,
    ctypes.POINTER(idx_t),
    ctypes.POINTER(idx_t),
    ctypes.POINTER(real_t),
    ctypes.POINTER(real_t),
    ctypes.POINTER(idx_t)
]
_metisddm.metis_partitioning.restype = None


def metis_partitioning(total_actors, cus, tasks_forecast, capacity, comm_matrix, anno_matrix, msg_exch_cost):
    global last_ddm_total_actors_invocation
    if len(tasks_forecast) != total_actors:
        raise ValueError(f"tasks_forecast should have {total_actors} elements, but it has {len(tasks_forecast)}")

    arr_tasks = (idx_t * total_actors)(*tasks_forecast)
    arr_capacity = (idx_t * cus)(*capacity)

    #print(tasks_forecast)

    # mat_comm_matrix = (ctypes.c_double * (total_actors * total_actors))  # Flat array version
    # mat_anno_matrix = (ctypes.c_double * (total_actors * total_actors))  # Flat array version

    # Convert each row to a ctypes array and flatten
    # flattened_comm_matrix = [ctypes.c_double(val) for row in comm_matrix for val in row]
    # flattened_anno_matrix = [ctypes.c_double(val) for row in anno_matrix for val in row]


    # Now assign the flattened arrays to the matrix
    # mat_comm_matrix = (ctypes.c_double * len(flattened_comm_matrix))(*flattened_comm_matrix)
    # mat_anno_matrix = (ctypes.c_double * len(flattened_anno_matrix))(*flattened_anno_matrix)

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
    _metisddm.metis_partitioning(total_actors, cus, arr_tasks, arr_capacity, arr_comm, arr_anno, arr_msg_exch)


_metisddm.metis_get_partitioning.argtypes = []
_metisddm.metis_get_partitioning.restype = ctypes.POINTER(idx_t)


def metis_get_partitioning():
    global last_ddm_total_actors_invocation
    part = _metisddm.metis_get_partitioning()
    if not part:
        return None
    part = [part[i] for i in range(last_ddm_total_actors_invocation)]

    return part
