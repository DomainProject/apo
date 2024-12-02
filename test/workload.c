#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ddm.h>

#define MAX_LPS 1024        // Maximum number of logical processes
#define MAX_EVENTS 1000     // Maximum number of events per LP
#define MAX_COMPUTE_UNITS 5 // Maximum number of compute units (e.g., CPUs, GPUs, FPGAs)


// Aggregation statistics for each logical process
typedef struct {
	unsigned total_events;        // Total events processed
	double total_exec_time;       // Sum of execution times of all events
	unsigned total_rollbacks;     // Number of rollbacks
	double avg_exec_time;         // Average execution time
	double exec_time_variance;    // Variance in execution time
	unsigned stragglers[MAX_LPS]; // Straggler events received from each LP
} LP_Stats;

// Logical Process (LP) structure
typedef struct {
	int id;                         // LP ID
	int compute_unit_id;            // ID of the compute unit assigned to this LP
	double current_timestamp;       // Logical time of the LP
	LP_Stats stats;                 // Statistics for this LP
	double event_times[MAX_EVENTS]; // Execution times of the events for variance calculation
} LogicalProcess;

// Compute Unit structure
typedef struct {
	int id;              // Compute unit ID
	enum cu_type type;   // Type of compute unit (e.g., CPU, GPU, FPGA)
	int events_executed; // Number of events executed in the current time window
} ComputeUnit;

// Global meta-simulation state
LogicalProcess lps[MAX_LPS];
ComputeUnit compute_units[MAX_COMPUTE_UNITS] = {{0, CPU, 0}, {1, CPU, 0}, {2, GPU, 0}, {3, FPGA, 0}, {4, CPU, 0}};
int num_compute_units = 5;

// Workload parameters
int num_LPs = 4;                 // Number of logical processes
int burstiness = 10;             // How bursty the events are per LP
double avg_exec_time = 5.0;      // Average event execution time
double exec_time_variance = 2.0; // Variance in event execution time
double rollback_prob = 0.05;     // Probability of rollback
int max_lookahead = 10;          // Lookahead window for events

// Time control parameters
double wall_clock_duration = 10.0;       // Wall-clock time to run the simulation (in seconds)
double max_simulation_timestamp = 100.0; // Maximum simulation time to stop at
int use_wall_clock_time = 1;             // 1 for wall clock-based stop, 0 for timestamp-based stop

// Generate random double within a range
double rand_double(double min, double max)
{
	return min + (rand() / (RAND_MAX / (max - min)));
}

// Assign a logical process to a compute unit
void assign_lp_to_compute_unit(LogicalProcess *lp, ComputeUnit compute_units[], int num_compute_units)
{
	lp->compute_unit_id = rand() % num_compute_units;
	printf("LP %d assigned to Compute Unit %d (%u)\n", lp->id, lp->compute_unit_id,
	    compute_units[lp->compute_unit_id].type);
}

// Simulate an event for a logical process with rollback probability
double simulate_event(int event_id, LogicalProcess *lp, int num_lps)
{
	double exec_time = rand_double(avg_exec_time - exec_time_variance, avg_exec_time + exec_time_variance);
	printf("LP %d, Event %d: execution time = %.2f units\n", lp->id, event_id, exec_time);

	// Check if rollback occurs

	if(rand_double(0.0, 1.0) < rollback_prob) {
		int sender_lp_id = rand() % num_lps; // Randomly select a sender LP as the source of the straggler
		printf("LP %d, Event %d: rollback triggered by straggler from LP %d\n", lp->id, event_id, sender_lp_id);
		lp->stats.total_rollbacks++;          // Increment rollback count for this LP
		lp->stats.stragglers[sender_lp_id]++; // Keep track of the origin of the straggler, to compute annoyance
	}

	return exec_time;
}

// Reset the statistics for an LP after each aggregation period
void reset_stats(LP_Stats *stats)
{
	stats->total_events = 0;
	stats->total_exec_time = 0.0;
	stats->total_rollbacks = 0;
	stats->avg_exec_time = 0.0;
	stats->exec_time_variance = 0.0;
}

// Reset compute unit statistics after each aggregation period
void reset_compute_unit_stats(ComputeUnit *compute_unit)
{
	compute_unit->events_executed = 0;
}

// Aggregate statistics for a logical process
void aggregate_stats(LogicalProcess *lp, int num_events_processed)
{
	LP_Stats *stats = &lp->stats;
	stats->total_events = num_events_processed;
	stats->avg_exec_time = stats->total_exec_time / num_events_processed;

	// Calculate variance in execution times
	double sum_sq_diff = 0.0;
	for(int i = 0; i < num_events_processed; i++) {
		double diff = lp->event_times[i] - stats->avg_exec_time;
		sum_sq_diff += diff * diff;
	}
	stats->exec_time_variance = sum_sq_diff / num_events_processed;
}

// Call the decision algorithm for the system (aggregating stats from all LPs)
void decision_algorithm(LogicalProcess lps[], int num_LPs, ComputeUnit compute_units[], int num_compute_units,
    double current_time)
{
	// TODO: call ddm_optimize() here and update test bench data structures accordingly

	printf("\n--- Decision Algorithm called at simulation time %.2f seconds ---\n", current_time);
	for(int i = 0; i < num_LPs; i++) {
		LP_Stats *stats = &lps[i].stats;
		printf("LP %d: Total events processed: %d\n", lps[i].id, stats->total_events);
		printf("LP %d: Total execution time: %.2f\n", lps[i].id, stats->total_exec_time);
		printf("LP %d: Total rollbacks: %d\n", lps[i].id, stats->total_rollbacks);
		printf("LP %d: Average execution time: %.2f\n", lps[i].id, stats->avg_exec_time);
		printf("LP %d: Execution time variance: %.2f\n", lps[i].id, stats->exec_time_variance);
		printf("LP %d: Assigned Compute Unit: %d (%u)\n", lps[i].id, lps[i].compute_unit_id,
		    compute_units[lps[i].compute_unit_id].type);

		printf("LP %d: Stragglers received: ", lps[i].id);
		for(int j = 0; j < num_LPs; j++) {
			if(lps[i].stats.stragglers[j] > 0) {
				printf("    From LP %d: %d events", j, lps[i].stats.stragglers[j]);
			}
		}
	}
	for(int i = 0; i < num_compute_units; i++) {
		printf("Compute Unit %d (%u): Events executed: %d\n", compute_units[i].id, compute_units[i].type,
		    compute_units[i].events_executed);
	}
	printf("--------------------------------------------\n\n");
}

// Generate a workload simulation with multiple logical processes
void generate_workload(void)
{
	for(int i = 0; i < num_LPs; i++) {
		lps[i].id = i;
		lps[i].current_timestamp = 0.0;
		memset(lps[i].stats.stragglers, 0, MAX_LPS * sizeof(unsigned));
		reset_stats(&lps[i].stats);
		assign_lp_to_compute_unit(&lps[i], compute_units, num_compute_units);
	}

	// TODO: call ddm_init() here

	clock_t start_time = clock(); // For wall-clock-based termination
	double simulated_time = 0.0;  // Total simulated time
	double event_time_step = 1.0; // Time step per event batch

	while(1) {
		for(int i = 0; i < num_LPs; i++) {
			LogicalProcess *lp = &lps[i];
			ComputeUnit *cu = &compute_units[lp->compute_unit_id];
			int events_in_time_step = rand() % burstiness + 1; // Generate events for this LP

			// Simulate events for this LP in the current time step
			for(int j = 0; j < events_in_time_step; j++) {
				double exec_time = simulate_event(j, lp, num_LPs);
				lp->event_times[j] = exec_time;
				lp->stats.total_exec_time += exec_time;
				cu->events_executed++;
			}

			// Aggregate statistics and update LP's logical time
			aggregate_stats(lp, events_in_time_step);
			lp->current_timestamp += event_time_step;
		}

		// Call the decision algorithm periodically
		if(simulated_time >= event_time_step) {
			decision_algorithm(lps, num_LPs, compute_units, num_compute_units, simulated_time);
			for(int i = 0; i < num_LPs; i++) {
				reset_stats(&lps[i].stats); // Reset stats for next window
				memset(lps[i].stats.stragglers, 0,
				    MAX_LPS * sizeof(unsigned)); // Reset straggler count for next window
			}
			for(int i = 0; i < num_compute_units; i++) {
				reset_compute_unit_stats(&compute_units[i]); // Reset compute unit stats for next window
			}
		}

		simulated_time += event_time_step;

		// Stop based on wall-clock time
		if(use_wall_clock_time) {
			double elapsed_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
			if(elapsed_time >= wall_clock_duration) {
				printf("Simulation stopped after %.2f seconds of wall-clock time\n", elapsed_time);
				break;
			}
		}
		// Stop based on simulation timestamp
		else {
			if(simulated_time >= max_simulation_timestamp) {
				printf("Simulation stopped at simulated time %.2f seconds\n", simulated_time);
				break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	// Seed random number generator
	srand(time(NULL));

	// Optional: read parameters from command line
	if(argc > 1)
		num_LPs = atoi(argv[1]);
	if(argc > 2)
		burstiness = atoi(argv[2]);
	if(argc > 3)
		avg_exec_time = atof(argv[3]);
	if(argc > 4)
		exec_time_variance = atof(argv[4]);
	if(argc > 5)
		rollback_prob = atof(argv[5]);
	if(argc > 6)
		wall_clock_duration = atof(argv[6]);
	if(argc > 7)
		max_simulation_timestamp = atof(argv[7]);
	if(argc > 8)
		use_wall_clock_time = atoi(argv[8]);

	// Ensure num_LPs doesn't exceed the maximum
	if(num_LPs > MAX_LPS) {
		printf("Error: Number of logical processes exceeds the maximum (%d)\n", MAX_LPS);
		return 1;
	}

	// Generate the workload
	printf("Generating workload with %d logical processes...\n", num_LPs);
	generate_workload();

	return 0;
}
