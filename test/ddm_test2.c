#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/ddm.h"

#define NCUS 4
#define NACT 10

int main(void)
{
	int total_cus = NCUS;
	int total_actors = NACT;
	enum cu_type cus[NCUS] = {1, 1, 2, 4};
	int msg_exch_cost[NCUS][NCUS] = {{1, 1, 5, 5}, {1, 1, 5, 5}, {3, 3, 7, 7}, {3, 3, 7, 7}};
	short runnable_on[NACT] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
	int cu_capacity[NCUS] = {4, 4, 2, 1};


	ddm_init(total_cus, total_actors, cus, msg_exch_cost, runnable_on);

	// <annoynace,msg_exchange_rate>
	struct actor_matrix actors[NACT][NACT] = {
	    {{1, 5}, {1, 5}, {0, 5}, {1, 6}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 6}, {1, 5}},
	    {{1, 5}, {1, 4}, {1, 5}, {1, 4}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}},
	    {{1, 3}, {1, 3}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 3}, {1, 4}},
	    {{1, 4}, {1, 4}, {1, 3}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 3}, {1, 4}},
	    {{1, 4}, {1, 4}, {1, 3}, {1, 4}, {1, 4}, {1, 3}, {1, 4}, {1, 4}, {1, 4}, {1, 4}},
	    {{1, 5}, {1, 5}, {1, 5}, {1, 5}, {0, 4}, {1, 4}, {1, 5}, {1, 5}, {1, 5}, {1, 4}},
	    {{1, 4}, {1, 3}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}},
	    {{1, 9}, {1, 8}, {1, 8}, {1, 9}, {1, 9}, {1, 8}, {1, 9}, {1, 6}, {1, 9}, {1, 10}},
	    {{1, 6}, {1, 5}, {1, 6}, {1, 6}, {1, 5}, {1, 5}, {1, 5}, {1, 6}, {1, 5}, {1, 5}},
	    {{1, 5}, {1, 5}, {1, 4}, {1, 5}, {1, 5}, {1, 4}, {1, 4}, {1, 5}, {1, 5}, {1, 4}}};

	int tasks_forecast[NACT] = {50, 50, 50, 50, 20, 50, 50, 80, 10, 10};

	ddm_optimize(total_actors, actors, tasks_forecast, total_cus, cu_capacity);
	int *res;

	while((res = ddm_poll()) == NULL) {
		printf(".");
	}
	printf("\n");

	for(int i = 0; i < total_actors; ++i) {
		printf("%2d -> %2d\n", i, res[i]);
	}
	free(res);

	return 0;
}
