#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <metis.h>
#include <math.h>
#include "utils.h"


#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))

static idx_t nEdges = 0;
idx_t actors = 0;
idx_t alpha = 0;


idx_t avg_edge_wgt = 0;
idx_t avg_vert_wgt = 0;

static idx_t *part_o = NULL;
static idx_t *part_a = NULL;
static idx_t *part_c = NULL;

static idx_t *final_part = NULL;

real_t comm_cost_matrix[NUM_ACTORS][NUM_ACTORS] = {0};
real_t anno_matrix[NUM_ACTORS][NUM_ACTORS] = {0};
idx_t msg_exch_cost[N_CUS][N_CUS] = {0};

void ddmmetis_init(idx_t total_actors)
{
	actors = total_actors;
}


void metis_init(idx_t act, idx_t n_cus, idx_t **xadj, idx_t **adjncy, idx_t **adjwgt,
    real_t communication_matrix[actors][actors], real_t annoyance_matrix[actors][actors], int aug)
{
	Edge *edges = createEdges(act, communication_matrix, annoyance_matrix, &nEdges);

	idx_t total_actors = (aug) ? act * n_cus : act;
	generateCSR(total_actors, nEdges, edges, xadj, adjncy, adjwgt);


	free(edges);
}

void update_weights(idx_t total_actors, idx_t *part, idx_t *xadj, idx_t *adjncy, idx_t **adjwgt, idx_t **vwgt, idx_t alpha) {



    long long sum_cost = 0;
    long max_edge_cost = 0;
    for (int i = 0; i < xadj[total_actors]; i++) {
    	sum_cost += (*adjwgt)[i];
    	if ((*adjwgt)[i] > max_edge_cost) max_edge_cost = (*adjwgt)[i];
    }

    
    double lambda = (double)(sum_cost - alpha) / (sum_cost + 1.0);  // Avoid divide by zero

	if (part != NULL) {
        for(int i = 0; i < total_actors; i++) {
            PRINTER() printf("neighbors of vertex %d :\t", i);
            for(int j = xadj[i]; j < xadj[i + 1]; j++) {

            	
                if(i != adjncy[j] && part[i] == part[adjncy[j]]) {

              
					PRINTER()
					printf(
					    "vertex %d and vertex %ld are on the same partition (%ld, %ld) ! Reward them -> (%ld, %ld) [alpha %ld] \n",
					    i, adjncy[j], part[i], part[adjncy[j]], (*adjwgt)[i], (*adjwgt)[adjncy[j]], alpha);
					
					(*adjwgt)[i] = max(1, round((*adjwgt)[i] * (1 + lambda)));
					(*adjwgt)[j] = max(1, round((*adjwgt)[j] * (1 + lambda)));
					
					PRINTER() printf("After update → adjwgt[i] %ld \t adjwgt[neighbor] %ld \t vwgt[i] %ld \t vwgt[neghbor] %ld \n",
								(*adjwgt)[i], (*adjwgt)[j], (*vwgt)[i], (*vwgt)[adjncy[j]] );
				

                } 
                PRINTER()
				printf("After update → adjwgt[i] %ld \t adjwgt[neighbor] %ld \t vwgt[i] %ld \t vwgt[neghbor] %ld \n",
								(*adjwgt)[i], (*adjwgt)[j], (*vwgt)[i], (*vwgt)[adjncy[j]] );
				
            }
            PRINTER() printf("\n");
			
        }
    }


}




void compute_partition_weights(idx_t *part, idx_t total_actors, idx_t **vwgt, idx_t cus, idx_t *partition_weights, idx_t *total_weight) {
    for (int i = 0; i < cus; i++) {
        partition_weights[i] = 0;
    }
    *total_weight = 0;
    for (int i = 0; i < total_actors; i++) {
        partition_weights[part[i]] += (*vwgt)[i];
        *total_weight += (*vwgt)[i];
    }
}

// Function to reassign vertices based on imbalance and minimize overload
void rebalance_partition(idx_t *part, idx_t total_actors, idx_t **vwgt, idx_t cus, real_t *capacity) {
    idx_t partition_weights[cus];
    idx_t total_weight;
    double ideal_weight;

    compute_partition_weights(part, total_actors, vwgt, cus, partition_weights, &total_weight);

    ideal_weight = total_weight / cus;

    if (part != NULL) {
    	
	    for (int i = 0; i < total_actors; i++) {
	        idx_t p = part[i]; // Partition of vertex i
	        double imbalance_ratio = (double) partition_weights[p] / ideal_weight - 1.0;

	        // Adjust weight based on imbalance ratio
	        if (imbalance_ratio > 0) {
	            // Overloaded partition → increase vertex weight to discourage placement
	            (*vwgt)[i] = max(1, round((*vwgt)[i] * (1 + imbalance_ratio * 1.0)));
	        } else {
	            // Underloaded partition → decrease vertex weight to encourage placement
	            (*vwgt)[i] = max(1, round((*vwgt)[i] * (1 + imbalance_ratio * 1.0)));
	        }
	    }

	    for (int i = 0; i < total_actors; i++) {
	        int p = part[i];
	        // Check if the partition is overloaded or underloaded
	        double imbalance_ratio = (double) partition_weights[p] / ideal_weight - 1.0;

	        if (imbalance_ratio > 0) {
	            // If the partition is overloaded, try moving the vertex to another partition
	            idx_t best_partition = p;
	            double min_diff = imbalance_ratio;
	            for (int j = 0; j < cus; j++) {
	                if (j != p && capacity[j] > 0.0f) {
	                    // Compute the impact of moving vertex i to partition j
	                    double temp_imbalance = (double) (partition_weights[j] + (*vwgt)[i]) / ideal_weight - 1.0;
	                    if (temp_imbalance < min_diff) {
	                        best_partition = j;
	                        min_diff = temp_imbalance;
	                    }
	                }
	            }

	            // Move the vertex to the best partition
	            if (best_partition != p) {
	                part[i] = best_partition;
	                partition_weights[p] -= (*vwgt)[i];
	                partition_weights[best_partition] += (*vwgt)[i];
	                capacity[best_partition] -= (double)(*vwgt)[i] / total_weight;
	                capacity[p] += (double)(*vwgt)[i] / total_weight;
	            }
	        }
	    }
	}
}

void metis_heterogeneous_multilevel(idx_t total_actors, idx_t n_cus, idx_t *tasks_forecast, idx_t *capacity,
    real_t input_comm_cost_matrix[actors][actors], real_t input_anno_matrix[actors][actors], real_t input_msg_exch_cost[n_cus][n_cus])
{
	idx_t nParts = n_cus; // Number of partitions (number of CUs)

	idx_t ubfactor = 30; // default imbalance tolerance
	// real_t tpwgts[cus] = {0.6, 0.3, 0.1};  // sum of elements must be 1


	idx_t *xadj, *adjncy, *adjwgt;
	idx_t *vwgt = calloc(total_actors, sizeof(idx_t)); // Vertex weights (used by metis to balance the partitions)
	memcpy(vwgt, tasks_forecast, sizeof(idx_t) * total_actors);
	/*for(int i = 0; i < total_actors; i++) {
		vwgt[i] = (tasks_forecast[i] == 0) ? 1 : tasks_forecast[i]; //zero-weights is problematic
	}*/

	if(input_comm_cost_matrix != NULL)
		memcpy(comm_cost_matrix, input_comm_cost_matrix, sizeof(comm_cost_matrix));
	if(input_anno_matrix != NULL)
		memcpy(anno_matrix, input_anno_matrix, sizeof(anno_matrix));

	if(input_msg_exch_cost != NULL)
		memcpy(msg_exch_cost, input_msg_exch_cost, sizeof(msg_exch_cost));

	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, NULL, anno_matrix, 0);


	update_weights(total_actors, part_o, xadj, adjncy, &adjwgt, &vwgt, alpha);
	

	PRINTER() printf("(first) avg edges %ld \t avg vertexes %ld\n", avg_edge_wgt, avg_vert_wgt);

	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


	PRINTER() printf("total_actors %ld \t cus %ld \n", total_actors, n_cus);


	PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE **** \n");
	compute_partition(total_actors, xadj, adjncy, NULL, NULL, adjwgt, nParts, NULL, NULL, ubfactor, &alpha, &part_a, 0);

	free(xadj);
	free(adjncy);
	free(adjwgt);

	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, comm_cost_matrix, NULL, 1);

	PRINTER() printf("(second) avg edges %ld \t avg vertexes %ld\n", avg_edge_wgt, avg_vert_wgt);

	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


	update_weights(total_actors, part_a, xadj, adjncy, &adjwgt, &vwgt, alpha);


	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


	idx_t aug_v = total_actors * n_cus;
	idx_t *new_adjwgt;
	idx_t *new_vwgt;

	idx_t *new_xadj = populate_newxadj(aug_v / n_cus, n_cus, xadj, vwgt, &new_vwgt);

	idx_t maxEdges = new_xadj[aug_v];

	PRINTER() printf("NVERTICES %ld \t MAXEDGES %ld\n", aug_v, maxEdges);

	idx_t *new_adjncy = populate_newadjncy(aug_v / n_cus, n_cus, maxEdges, xadj, new_xadj, adjncy, adjwgt, &new_adjwgt, msg_exch_cost);

	PRINTER() print_csr_graph(aug_v, new_xadj, new_adjncy, new_vwgt, new_adjwgt);

	ubfactor = 50;

    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE AND COMMUNICATION COST **** \n");
    compute_partition(aug_v, new_xadj, new_adjncy, new_vwgt, NULL, new_adjwgt, nParts, NULL, NULL, ubfactor, &alpha, &part_c, 1);

    free(xadj);
	free(adjncy);
	free(adjwgt);

	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, comm_cost_matrix, NULL, 0);

	PRINTER() printf("(third) avg edges %ld \t avg vertexes %ld\n", avg_edge_wgt, avg_vert_wgt);	

	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


	update_weights(total_actors, part_c, xadj, adjncy, &adjwgt, &vwgt, alpha);


	ubfactor = 100;

	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);

    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE AND COMMUNICATION COST AND OVERLOAD **** \n");
    compute_partition(total_actors, xadj, adjncy, vwgt, NULL, adjwgt, nParts, NULL, NULL, ubfactor, &alpha, &part_o, 0);

	PRINTER() print_partition(total_actors, part_o);
    


    rebalance_partition(part_o, total_actors, &vwgt, n_cus, capacity);

	PRINTER() print_partition(total_actors, part_o);


	if (final_part == NULL) {
		final_part = malloc(total_actors * sizeof(idx_t));

		if (final_part == NULL) {
	        printf("Memory allocation failed for final_part.\n");
	        exit(1); 
	    }
	}

	memcpy(final_part, part_o, total_actors * sizeof(idx_t));

	free(new_xadj);
	free(new_adjncy);
	free(new_vwgt);
	free(new_adjwgt);

	free(xadj);
	free(adjncy);
	free(adjwgt);
	free(vwgt);


}




void metis_communication(idx_t total_actors, idx_t n_cus, idx_t *tasks_forecast,
    real_t input_comm_cost_matrix[actors][actors], real_t input_msg_exch_cost[n_cus][n_cus])
{
	idx_t nParts = n_cus; // Number of partitions (number of CUs)

	idx_t ubfactor = 30; // default imbalance tolerance

	idx_t *xadj, *adjncy, *adjwgt;
	idx_t *vwgt = calloc(total_actors, sizeof(idx_t)); // Vertex weights (used by metis to balance the partitions)
	memcpy(vwgt, tasks_forecast, sizeof(idx_t) * total_actors);


	if(input_comm_cost_matrix != NULL)
		memcpy(comm_cost_matrix, input_comm_cost_matrix, sizeof(comm_cost_matrix));
	
	if(input_msg_exch_cost != NULL)
		memcpy(msg_exch_cost, input_msg_exch_cost, sizeof(msg_exch_cost));

	///FIRST ROUND OF PARTITIONING 
	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, comm_cost_matrix, NULL, 0);

	compute_partition(total_actors, xadj, adjncy, vwgt, NULL, NULL, nParts, NULL, NULL, ubfactor, &alpha, &part_o, 0);


	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, comm_cost_matrix, NULL, 1);


	update_weights(total_actors, part_o, xadj, adjncy, &adjwgt, &vwgt, alpha);

	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);

	idx_t aug_v = total_actors * n_cus;
	idx_t *new_adjwgt;
	idx_t *new_vwgt;

	idx_t *new_xadj = populate_newxadj(aug_v / n_cus, n_cus, xadj, vwgt, &new_vwgt);

	idx_t maxEdges = new_xadj[aug_v];

	PRINTER() printf("NVERTICES %ld \t MAXEDGES %ld\n", aug_v, maxEdges);

	idx_t *new_adjncy = populate_newadjncy(aug_v / n_cus, n_cus, maxEdges, xadj, new_xadj, adjncy, adjwgt, &new_adjwgt, msg_exch_cost);

	PRINTER() print_csr_graph(aug_v, new_xadj, new_adjncy, new_vwgt, new_adjwgt);

	ubfactor = 50;

    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE AND COMMUNICATION COST **** \n");
    compute_partition(aug_v, new_xadj, new_adjncy, new_vwgt, NULL, new_adjwgt, nParts, NULL, NULL, ubfactor, &alpha, &part_c, 1);

	PRINTER() print_partition(total_actors, part_c);

	final_part = part_c;

	free(xadj);
	free(adjncy);
	free(adjwgt);
	free(vwgt);

	free(new_xadj);
	free(new_adjncy);
	free(new_vwgt);
	free(new_adjwgt);
}

void metis_baseline(idx_t total_actors, idx_t n_cus, idx_t *tasks_forecast, real_t input_comm_cost_matrix[actors][actors], real_t input_tpwgts[n_cus])
{
	idx_t nParts = n_cus; // Number of partitions (number of CUs)


	idx_t ubfactor = 30; // default imbalance tolerance

	idx_t *xadj, *adjncy, *adjwgt;
	idx_t *vwgt = calloc(total_actors, sizeof(idx_t)); // Vertex weights (used by metis to balance the partitions)
	memcpy(vwgt, tasks_forecast, sizeof(idx_t) * total_actors);

	real_t *tpwgts = calloc(n_cus, sizeof(real_t));
	memcpy(tpwgts, input_tpwgts, sizeof(real_t)*n_cus);

	real_t sum_tpwgts = 0.;
	for (int i=0; i < n_cus; i++) sum_tpwgts += tpwgts[i];
	if (sum_tpwgts != 1.0f) {
		tpwgts = NULL; /// just to make sure that it is not actually used
		fprintf(stderr, "Sum of tpwgts is not 1! Cannot be used in METIS, setting it to NULL\n");
	}

	if(input_comm_cost_matrix != NULL)
		memcpy(comm_cost_matrix, input_comm_cost_matrix, sizeof(comm_cost_matrix));
	
	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, comm_cost_matrix, NULL, 0);


	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);

    compute_partition(total_actors, xadj, adjncy, vwgt, NULL, adjwgt, nParts, tpwgts, NULL, ubfactor, &alpha, &part_o, 0);




	PRINTER() print_partition(total_actors, part_o);

	final_part = part_o;


	free(xadj);
	free(adjncy);
	free(adjwgt);
	free(vwgt);

}




idx_t *metis_get_partitioning(void)
{
	PRINTER() printf("*** PARTITION TO BE INSTALLED **** \n");
	return final_part;
}
