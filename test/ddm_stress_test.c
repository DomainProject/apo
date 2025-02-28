#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/ddm.h"
#include <time.h>
#include <float.h>

#define NCUS 4
#define NACT 8

#define ROLLBACK_PROBABILITY 0.1
#define TOTAL_MESSAGES 10000
#define ROLLBACK_COST 10

#define TOLERANCE_FACTOR 1.2 // Used to see if a DDM assignment is acceptable

static enum cu_type cus[NCUS] = {1, 1, 2, 4};
static int msg_exch_cost[NCUS][NCUS] = {
    //        C  C  G  F
    /*CPU*/ {1, 1, 2, 2},
    /*CPU*/ {1, 1, 2, 2},
    /*GPU*/ {2, 2, 4, 4},
    /*FPGA*/ {2, 2, 4, 4}};
static short runnable_on[NACT] = {7, 7, 7, 7, 7, 7, 7, 7};
static int cu_capacity[NCUS] = {100, 100, 100, 100};


// Used by quicksort below
static int cmp_int(const void *a, const void *b)
{
	int int_a = *(const int *)a;
	int int_b = *(const int *)b;
	return int_a - int_b;
}


/*
   Divide a total number of events into a given number of parts.
   It generates 'parts - 1' random cut points within the interval [0, total],
   sorts these cuts, and then computes differences between successive cut points.
   This procedure yields a random composition of total events and is a well‐established
   technique in randomized algorithms
*/
static void partition_total(int total, int parts, int out[])
{
	if(parts <= 0)
		return;
	if(parts == 1) {
		out[0] = total;
		return;
	}
	int *cuts = malloc((parts - 1) * sizeof(int));
	if(!cuts) {
		fprintf(stderr, "Memory allocation error in partition_total\n");
		exit(EXIT_FAILURE);
	}
	for(int i = 0; i < parts - 1; i++) {
		cuts[i] = rand() % (total + 1);
	}
	qsort(cuts, parts - 1, sizeof(int), cmp_int);
	out[0] = cuts[0];
	for(int i = 1; i < parts - 1; i++) {
		out[i] = cuts[i] - cuts[i - 1];
	}
	out[parts - 1] = total - cuts[parts - 2];
	free(cuts);
}


/*
   generate_random_tasks_forecast partitions TOTAL_EVENTS among the NACT actors.
   The resulting tasks_forecast array holds non-negative integers that sum to TOTAL_EVENTS.
   This method guarantees a consistent total workload distributed among the actors,
   a common requirement in simulation-based evaluations
*/
static void generate_random_tasks_forecast(int tasks_forecast[NACT])
{
	partition_total(TOTAL_MESSAGES, NACT, tasks_forecast);
}


/*
   For each actor i, the total number of events
   (tasks) that actor is expected to process is given by tasks_forecast[i]. This value is
   partitioned across the NACT entries in the i-th row to set the message exchange rates,
   ensuring that the sum of events for actor i in the matrix equals its tasks forecast.
   The maximum potential annoyance for each actor is computed as a fraction of its forecast,
   so that the rollback probability scales with the workload. This coherent design ties together
   the workload distribution and inter-actor communication in a single framework
*/
static void generate_random_actor_matrix(struct actor_matrix actors[NACT][NACT], const int tasks_forecast[NACT])
{
	int partition[NACT];
	for(int i = 0; i < NACT; i++) {
		partition_total(tasks_forecast[i], NACT, partition);
		/* The maximum annoyance for actor i is proportional to its workload */
		int max_annoyance = (int)(ROLLBACK_PROBABILITY * tasks_forecast[i]);
		for(int j = 0; j < NACT; j++) {
			actors[i][j].msg_exchange_rate = partition[j];
			actors[i][j].annoyance = rand() % (max_annoyance + 1);
		}
	}
}


/*
   evaluate_assignment computes a composite cost metric for a given assignment.
   The cost aggregates the communication cost derived from the actor matrix and the rollback
   penalty (scaled by ROLLBACK_COST), along with a term that reflects the load relative to
   compute unit capacity. This integrated measure is instrumental in assessing the efficiency
   of an assignment
*/
static double evaluate_assignment(const int *assignment, struct actor_matrix actors[NACT][NACT],
    const int tasks_forecast[NACT])
{
	double cost = 0.0;
	for(int i = 0; i < NACT; i++) {
		for(int j = 0; j < NACT; j++) {
			int cu_i = assignment[i];
			int cu_j = assignment[j];
			cost += actors[i][j].msg_exchange_rate * msg_exch_cost[cu_i][cu_j];
			if(cu_i != cu_j) {
				cost += actors[i][j].annoyance * ROLLBACK_COST;
			}
		}
	}

	// Compute the load per compute unit.
	int load[NCUS] = {0};
	for (int i = 0; i < NACT; i++) {
		load[assignment[i]] += tasks_forecast[i];
	}

	// Introduce a stronger, superlinear overload penalty.
	// Rather than simply summing a quadratic penalty, we use a cubic penalty
	// to more strongly discourage overload and uneven distribution.
	double load_cost = 0.0;
	for (int cu = 0; cu < NCUS; cu++) {
		double ratio = (double)load[cu] / (double)cu_capacity[cu];
		// The cubic term sharply increases as ratio grows above 1.
		load_cost += ratio * ratio * ratio;
	}

	// Combine communication, rollback, and load imbalance costs.
	cost += load_cost;
	return cost;
}


/*
   Generate all possible assignments of NACT actors to NCUS compute units and
   keep track of the minimum cost found.
*/
static void rec_enum_assignments(int idx, int assignment[NACT], struct actor_matrix actors[NACT][NACT],
    const int tasks_forecast[NACT], double *min_cost)
{
	if(idx == NACT) {
		double current_cost = evaluate_assignment(assignment, actors, tasks_forecast);
		if(current_cost < *min_cost) {
			*min_cost = current_cost;
		}
		return;
	}
	for(int cu = 0; cu < NCUS; cu++) {
		assignment[idx] = cu;
		rec_enum_assignments(idx + 1, assignment, actors, tasks_forecast, min_cost);
	}
}


/*
    compares the cost of a found assignment to the minimum cost reference
    (computed by enumeration) scaled by a tolerance factor. If the found cost exceeds
    the reference by more than TOLERANCE_FACTOR, the assignment is flagged as suboptimal.
*/
static bool is_cost_acceptable(double cost, struct actor_matrix actors[8][8], const int tasks_forecast[NACT], double *ref_cost)
{
	// Enumerate all possible assignments and test them with this configuration.
	int assignment[NACT] = {0};
	*ref_cost = DBL_MAX;

	rec_enum_assignments(0, assignment, actors, tasks_forecast, ref_cost);

	if(cost > *ref_cost * TOLERANCE_FACTOR) {
		printf("Assignment flagged as suboptimal (exceeds reference cost by factor %.2f).\n", TOLERANCE_FACTOR);
		printf("Reference minimum cost: %.2f\n", *ref_cost);
		printf("Minimum cost found with this assignment:\n");
		for(int i = 0; i < NACT; ++i) {
			printf("%2d -> %2d\n", i, assignment[i]);
		}
		return false;
	} else {
		return true;
	}
}



int main(int argc, char **argv)
{
	// <annoyance,msg_exchange_rate>
	struct actor_matrix actors[NACT][NACT];
	int tasks_forecast[NACT];
	int *assignment;

	time_t seed = time(NULL);

	if(argc == 2) {
		seed = atol(argv[1]);
	}
	srand(seed);
	ddm_init(NCUS, NACT, cus, msg_exch_cost, runnable_on);

	generate_random_tasks_forecast(tasks_forecast);
	generate_random_actor_matrix(actors, tasks_forecast);
	ddm_optimize(NACT, actors, tasks_forecast, NCUS, cu_capacity);
	while((assignment = ddm_poll()) == NULL)
		;
	double total_cost = evaluate_assignment(assignment, actors, tasks_forecast);
	double reference_cost = DBL_MAX;

	if(!is_cost_acceptable(total_cost, actors, tasks_forecast, &reference_cost)) {
		printf("Cost with the found assignment: %.2f\n", total_cost);
		for(int i = 0; i < NACT; ++i) {
			printf("%2d -> %2d\n", i, assignment[i]);
		}
		printf("Seed: %ld\n", seed);

		// Print actors and task forecast
		printf("Actor matrix:\n");
		for(int i = 0; i < NACT; i++) {
			for(int j = 0; j < NACT; j++) {
				printf("(%d, %d) ", actors[i][j].annoyance, actors[i][j].msg_exchange_rate);
			}
			printf("\n");
		}
		printf("Tasks forecast:\n");
		for(int i = 0; i < NACT; i++)
			printf("%d ", tasks_forecast[i]);
		printf("\n");
	} else {
		printf("Assignment with cost %.2f is acceptable (reference cost is %.2f).\n", total_cost, reference_cost);
	}

	free(assignment);

	return 0;
}
