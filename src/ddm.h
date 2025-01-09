#include "clingo_solver.h"

// cu
enum cu_type { CPU = 1, GPU = 2, FPGA = 4 };

extern int num_actors;
struct actor_matrix {
	int annoyance;
	int msg_exchange_rate;
};

extern void ddm_init(int total_cus, int total_actors, enum cu_type *cus, int msg_exch_cost[total_cus][total_cus],
    short runnable_on[total_actors]);
extern void ddm_optimize(int total_actors, struct actor_matrix actors[total_actors][total_actors],
    int tasks_forecast[total_actors], int total_cus, int cu_capacity[total_cus], clingo_ctx **cctx);
extern int *ddm_poll(int total_actors, bool stop_on_optimal, clingo_ctx *cctx);
extern void ddm_free(clingo_ctx *cctx);
