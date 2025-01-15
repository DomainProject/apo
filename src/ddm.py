import ctypes


class actor_matrix(ctypes.Structure):
    _fields_ = [
        ("annoyance", ctypes.c_int),
        ("msg_exchange_rate", ctypes.c_int)
    ]


_domainddm = ctypes.CDLL('../cmake-build-debug/src/libdomainddm.so')

_domainddm.ddm_init.argtypes = [
    ctypes.c_int,
    ctypes.c_int,
    ctypes.POINTER(ctypes.c_int),
    ctypes.POINTER(ctypes.c_int),
    ctypes.POINTER(ctypes.c_short)
]
_domainddm.ddm_init.restype = None


def ddm_init(total_cus, total_actors, cus, msg_exch_cost, runnable_on):
    arr_cus = (ctypes.c_int * total_cus)(*cus)
    flattened = []
    for row in msg_exch_cost:
        flattened.extend(row)
    arr_msg_exch = (ctypes.c_int * (total_cus * total_cus))(*flattened)
    arr_run = (ctypes.c_short * total_actors)(*runnable_on)
    _domainddm.ddm_init(total_cus, total_actors, arr_cus, arr_msg_exch, arr_run)


_domainddm.ddm_optimize.argtypes = [
    ctypes.c_int,
    ctypes.POINTER(actor_matrix),
    ctypes.POINTER(ctypes.c_int),
    ctypes.c_int,
    ctypes.POINTER(ctypes.c_int)
]
_domainddm.ddm_optimize.restype = None


def ddm_optimize(total_actors, actors, tasks_forecast, total_cus, cu_capacity):
    flattened_actors = []
    for row in actors:
        flattened_actors.extend(row)
    arr_actors = (actor_matrix * (total_actors * total_actors))(*flattened_actors)
    arr_tasks = (ctypes.c_int * total_actors)(*tasks_forecast)
    arr_capacity = (ctypes.c_int * total_cus)(*cu_capacity)
    _domainddm.ddm_optimize(total_actors, arr_actors, arr_tasks, total_cus, arr_capacity)


# extern int *ddm_poll(void);
_domainddm.ddm_poll.argtypes = []
_domainddm.ddm_poll.restype = ctypes.POINTER(ctypes.c_int)


def ddm_poll():
    return _domainddm.ddm_poll()
