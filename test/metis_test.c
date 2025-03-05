#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/ddmmetis.h"
#include "../src/ddmmetis/utils.h"

#define NUM_ACTORS 10

real_t comm_cost[NUM_ACTORS][NUM_ACTORS] = {{5.f, 5.f, 5.f, 6.f, 5.f, 5.f, 5.f, 5.f, 6.f, 5.f},
    {5.f, 4.f, 5.f, 4.f, 5.f, 5.f, 5.f, 5.f, 5.f, 5.f}, {3.f, 3.f, 4.f, 4.f, 4.f, 4.f, 4.f, 4.f, 3.f, 4.f},
    {4.f, 4.f, 3.f, 4.f, 4.f, 4.f, 4.f, 4.f, 3.f, 4.f}, {4.f, 4.f, 3.f, 4.f, 4.f, 3.f, 4.f, 4.f, 4.f, 4.f},
    {5.f, 5.f, 5.f, 5.f, 4.f, 4.f, 5.f, 5.f, 5.f, 4.f}, {4.f, 3.f, 4.f, 4.f, 4.f, 4.f, 4.f, 4.f, 4.f, 4.f},
    {9, 8.f, 8.f, 9, 9, 8.f, 9, 6.f, 9, 10.f}, {6.f, 5.f, 6.f, 6.f, 5.f, 5.f, 5.f, 6.f, 5.f, 5.f},
    {5.f, 5.f, 4.f, 5.f, 5.f, 4.f, 4.f, 5.f, 5.f, 4.f}};

real_t anno[NUM_ACTORS][NUM_ACTORS] = {{1.f, 1.f, 0.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
    {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f}, {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
    {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f}, {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
    {1.f, 1.f, 1.f, 1.f, 0.f, 1.f, 1.f, 1.f, 1.f, 1.f}, {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
    {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f}, {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
    {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f}};

real_t msg_cost[N_CUS][N_CUS] = {
        {1, 1, 5, 5},
        {1, 1, 5, 5},
        {3, 3, 7, 7},
        {3, 3, 7, 7}
    };

int main(void)
{
	ddmmetis_init(NUM_ACTORS);

	idx_t tasks_forecast[NUM_ACTORS] = {50, 50, 50, 50, 20, 50, 50, 80, 10, 10};
	idx_t capacity[N_CUS] = {4, 4, 2, 1};

	metis_heterogeneous_multilevel(NUM_ACTORS, N_CUS, tasks_forecast, capacity, comm_cost, anno, msg_cost);
	idx_t *res = metis_get_partitioning();

	for(int i = 0; i < NUM_ACTORS; ++i) {
		printf("%2d -> %2ld\n", i, res[i]);
	}
	free(res);

	return 0;
}
