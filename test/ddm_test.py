from src.ddm import ddm_prepare_actor_matrix, ddm_init, ddm_optimize, ddm_poll


def test_ddm():

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

    ddm_init(total_cus, total_actors, cus, msg_exch_cost, runnable_on)

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

    # Convert tuples to actor_matrix structures
    act_struct = ddm_prepare_actor_matrix(actors)

    tasks_forecast = [50, 50, 50, 50, 20, 50, 50, 80, 10, 10]

    ddm_optimize(total_actors, act_struct, tasks_forecast, total_cus, cu_capacity)
    res = None
    while res is None:
        res = ddm_poll()
    result_values = [res[i] for i in range(total_actors)]
    expected = [3, 0, 3, 0, 3, 2, 3, 3, 3, 0]
    assert result_values == expected
