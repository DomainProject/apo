#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/ddm.h"

#define NCUS 25
#define NACT 8

int main(void)
{
	int total_cus = NCUS;
	int total_actors = NACT;
	enum cu_type cus[NCUS] = {1, 1, 2, 4};
	int msg_exch_cost[NCUS][NCUS] = {
	    //        C  C  G  F
	    /*CPU*/  {1, 1, 3, 3},
	    /*CPU*/  {1, 1, 3, 3},
	    /*GPU*/  {3, 3, 1, 5},
	    /*FPGA*/ {3, 3, 5, 1}
	};
	short runnable_on[NACT] = {7, 7, 7, 7, 7, 7, 7, 7};
	int cu_capacity[NCUS] = {46768, 44105, 44103, 44104};
	int tasks_forecast[NACT] = {23225, 21836, 21833, 21835, 23080, 21833, 21834, 21833};

	ddm_init(total_cus, total_actors, cus, msg_exch_cost, runnable_on);

	// <annoynace,msg_exchange_rate>
	struct actor_matrix actors[NACT][NACT] = {
	    {{0, 22551}, {480, 1920}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 21356}, {480, 1919}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 21352}, {479, 1919}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 21355}, {525, 1920}, {0, 0}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 22555}, {479, 1920}, {0, 0}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 21353}, {480, 1919}, {0, 0}},
	    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 21354}, {479, 1919}},
	    {{675, 1920}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 21354}}
	};

	ddm_optimize(total_actors, actors, tasks_forecast, total_cus, cu_capacity);
	int *res;

	while((res = ddm_poll()) == NULL);

	for(int i = 0; i < total_actors; ++i) {
		printf("%2d -> %2d\n", i, res[i]);
	}
	free(res);

	return 0;
}
