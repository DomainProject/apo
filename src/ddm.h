#pragma once

// cu
enum cu_type { CPU = 1, GPU = 2, FPGA = 4 };

struct actor_matrix {
	int annoyance;
	int msg_exchange_rate;
};

enum result {SEARCHING, UNSAT, FOUND};

extern void ddm_init(int total_cus, int total_actors, const enum cu_type *cus, int msg_exch_cost[total_cus][total_cus],
    short runnable_on[total_actors]);
extern void ddm_optimize(int total_actors, struct actor_matrix actors[total_actors][total_actors],
    int tasks_forecast[total_actors], int total_cus, int cu_capacity[total_cus] );
extern enum result ddm_poll(int **assignment);
extern void ddm_free_assignment(int *assignment);

extern void ddm_destroy(void);
