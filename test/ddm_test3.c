#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/ddm.h"

#define NCUS 4
#define NACT 8

int main(void)
{
	int total_cus = NCUS;
	int total_actors = NACT;
	enum cu_type cus[NCUS] = {1, 1, 2, 4};
	int msg_exch_cost[NCUS][NCUS] = {
	    //        C  C  G  F
	    /*CPU*/  {1, 1, 2, 2},
	    /*CPU*/  {1, 1, 2, 2},
	    /*GPU*/  {2, 2, 4, 4},
	    /*FPGA*/ {2, 2, 4, 4}
	};
	short runnable_on[NACT] = {7, 7, 7, 7, 7, 7, 7, 7};
	int cu_capacity[NCUS] = {1, 1, 1, 1};


	ddm_init(total_cus, total_actors, cus, msg_exch_cost, runnable_on);

	// <annoynace,msg_exchange_rate>
	struct actor_matrix actors[NACT][NACT] = {
	    {{0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 1}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 1}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 1}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 1}}
	};

	int tasks_forecast[NACT] = {10, 10, 10, 10, 10, 10, 10, 10};

	ddm_optimize(total_actors, actors, tasks_forecast, total_cus, cu_capacity);
	int *res;

	while((res = ddm_poll()) == NULL)
		;

	for(int i = 0; i < total_actors; ++i) {
		printf("%2d -> %2d\n", i, res[i]);
	}
	free(res);

	return 0;
}
