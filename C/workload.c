#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_LPS 10  // Maximum number of logical processes
#define MAX_EVENTS 1000  // Maximum number of events per LP

// Workload parameters
int num_LPs = 4;       // Number of logical processes
int burstiness = 10;    // How bursty the events are per LP
double avg_exec_time = 5.0;  // Average event execution time
double exec_time_variance = 2.0;  // Variance in event execution time
double rollback_prob = 0.05;  // Probability of rollback
int max_lookahead = 10;  // Lookahead window for events

// Time control parameters
double wall_clock_duration = 10.0;  // Wall-clock time to run the simulation (in seconds)
double max_simulation_timestamp = 100.0;  // Maximum simulation time to stop at
int use_wall_clock_time = 1;  // 1 for wall clock-based stop, 0 for timestamp-based stop

// Aggregation statistics for each logical process
typedef struct {
    int total_events;           // Total events processed
    double total_exec_time;     // Sum of execution times of all events
    int total_rollbacks;        // Number of rollbacks
    double avg_exec_time;       // Average execution time
    double exec_time_variance;  // Variance in execution time
} LP_Stats;

// Logical Process (LP) structure
typedef struct {
    int id;                     // LP ID
    double current_timestamp;    // Logical time of the LP
    LP_Stats stats;             // Statistics for this LP
    double event_times[MAX_EVENTS];  // Execution times of the events for variance calculation
} LogicalProcess;

// Generate random double within a range
double rand_double(double min, double max) {
    return min + (rand() / (RAND_MAX / (max - min)));
}

// Simulate an event for a logical process with rollback probability
double simulate_event(int event_id, LogicalProcess *lp) {
    double exec_time = rand_double(avg_exec_time - exec_time_variance, avg_exec_time + exec_time_variance);
    printf("LP %d, Event %d: execution time = %.2f units\n", lp->id, event_id, exec_time);

    // Check if rollback occurs
    if (rand_double(0.0, 1.0) < rollback_prob) {
        printf("LP %d, Event %d: rollback triggered\n", lp->id, event_id);
        lp->stats.total_rollbacks++;  // Increment rollback count for this LP
    }

    return exec_time;
}

// Reset the statistics for an LP after each aggregation period
void reset_stats(LP_Stats *stats) {
    stats->total_events = 0;
    stats->total_exec_time = 0.0;
    stats->total_rollbacks = 0;
    stats->avg_exec_time = 0.0;
    stats->exec_time_variance = 0.0;
}

// Aggregate statistics for a logical process
void aggregate_stats(LogicalProcess *lp, int num_events_processed) {
    LP_Stats *stats = &lp->stats;
    stats->total_events = num_events_processed;
    stats->avg_exec_time = stats->total_exec_time / num_events_processed;

    // Calculate variance in execution times
    double sum_sq_diff = 0.0;
    for (int i = 0; i < num_events_processed; i++) {
        double diff = lp->event_times[i] - stats->avg_exec_time;
        sum_sq_diff += diff * diff;
    }
    stats->exec_time_variance = sum_sq_diff / num_events_processed;
}

// Call the decision algorithm for the whole system (aggregating stats from all LPs)
void decision_algorithm(LogicalProcess lps[], int num_LPs, double current_time) {
    printf("\n--- Decision Algorithm called at simulation time %.2f seconds ---\n", current_time);
    for (int i = 0; i < num_LPs; i++) {
        LP_Stats *stats = &lps[i].stats;
        printf("LP %d: Total events processed: %d\n", lps[i].id, stats->total_events);
        printf("LP %d: Total execution time: %.2f\n", lps[i].id, stats->total_exec_time);
        printf("LP %d: Total rollbacks: %d\n", lps[i].id, stats->total_rollbacks);
        printf("LP %d: Average execution time: %.2f\n", lps[i].id, stats->avg_exec_time);
        printf("LP %d: Execution time variance: %.2f\n", lps[i].id, stats->exec_time_variance);
    }
    printf("--------------------------------------------\n\n");
}

// Generate a workload simulation with multiple logical processes
void generate_workload() {
    LogicalProcess lps[MAX_LPS];
    for (int i = 0; i < num_LPs; i++) {
        lps[i].id = i;
        lps[i].current_timestamp = 0.0;
        reset_stats(&lps[i].stats);
    }

    clock_t start_time = clock();  // For wall-clock-based termination
    double simulated_time = 0.0;   // Total simulated time
    double event_time_step = 1.0;  // Time step per event batch

    while (1) {
        for (int i = 0; i < num_LPs; i++) {
            LogicalProcess *lp = &lps[i];
            int events_in_time_step = rand() % burstiness + 1;  // Generate events for this LP

            // Simulate events for this LP in the current time step
            for (int j = 0; j < events_in_time_step; j++) {
                double exec_time = simulate_event(j, lp);
                lp->event_times[j] = exec_time;
                lp->stats.total_exec_time += exec_time;
            }

            // Aggregate statistics and update LP's logical time
            aggregate_stats(lp, events_in_time_step);
            lp->current_timestamp += event_time_step;
        }

        // Call the decision algorithm periodically
        if (simulated_time >= event_time_step) {
            decision_algorithm(lps, num_LPs, simulated_time);
            for (int i = 0; i < num_LPs; i++) {
                reset_stats(&lps[i].stats);  // Reset stats for next window
            }
        }

        simulated_time += event_time_step;

        // Stop based on wall-clock time
        if (use_wall_clock_time) {
            double elapsed_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            if (elapsed_time >= wall_clock_duration) {
                printf("Simulation stopped after %.2f seconds of wall-clock time\n", elapsed_time);
                break;
            }
        } 
        // Stop based on simulation timestamp
        else {
            if (simulated_time >= max_simulation_timestamp) {
                printf("Simulation stopped at simulated time %.2f seconds\n", simulated_time);
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // Seed random number generator
    srand(time(NULL));

    // Optional: read parameters from command line
    if (argc > 1) num_LPs = atoi(argv[1]);
    if (argc > 2) burstiness = atoi(argv[2]);
    if (argc > 3) avg_exec_time = atof(argv[3]);
    if (argc > 4) exec_time_variance = atof(argv[4]);
    if (argc > 5) rollback_prob = atof(argv[5]);
    if (argc > 6) wall_clock_duration = atof(argv[6]);
    if (argc > 7) max_simulation_timestamp = atof(argv[7]);
    if (argc > 8) use_wall_clock_time = atoi(argv[8]);

    // Ensure num_LPs doesn't exceed the maximum
    if (num_LPs > MAX_LPS) {
        printf("Error: Number of logical processes exceeds the maximum (%d)\n", MAX_LPS);
        return 1;
    }

    // Generate the workload
    printf("Generating workload with %d logical processes...\n", num_LPs);
    generate_workload();

    return 0;
}

