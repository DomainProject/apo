#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <metis.h>
#include "utils.h"

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a <= _b ? _a : _b;       \
})

static idx_t nEdges = 0;
idx_t actors = 0;
idx_t alpha = 0;


idx_t avg_edge_wgt = 0;
idx_t avg_vert_wgt = 0;

static idx_t *part_o = NULL;
static idx_t *part_a = NULL;
static idx_t *part_c = NULL;

real_t comm_cost_matrix[NUM_ACTORS][NUM_ACTORS] = {0};
real_t anno_matrix[NUM_ACTORS][NUM_ACTORS] = {0};
idx_t msg_exch_cost[N_CUS][N_CUS] = {0};

void ddmmetis_init(idx_t total_actors, idx_t total_cus)
{
	actors = total_actors;
}


void metis_init(idx_t act, idx_t n_cus, idx_t **xadj, idx_t **adjncy, idx_t **adjwgt, idx_t **vwgt,
    real_t communication_matrix[actors][actors], real_t annoyance_matrix[actors][actors], int aug)
{
	Edge *edges = createEdges(act, communication_matrix, annoyance_matrix, &nEdges);

	idx_t total_actors = (aug) ? act * n_cus : act;
	generateCSR(total_actors, nEdges, edges, xadj, adjncy, adjwgt);

	idx_t sum_vert_wgt = 0;
	for(int i = 0; i < act; i++) {
		sum_vert_wgt += (*vwgt)[i];
	}

	idx_t sum_edge_wgt = 0;
	for(int i = 0; i < (*xadj)[act]; i++) {
		sum_edge_wgt += (*adjwgt)[i];
	}

	PRINTER() printf("sum_edge_wgt %d \t sum_vert_wgt %ld\n", sum_edge_wgt, sum_vert_wgt);
	avg_edge_wgt = ((*xadj)[act] != 0) ? sum_edge_wgt / (*xadj)[act] : 0;
	avg_vert_wgt = sum_vert_wgt / total_actors;


	free(edges);
}

void update_weights(idx_t total_actors, idx_t *part, idx_t *xadj, idx_t *adjncy, idx_t **adjwgt, idx_t **vwgt, idx_t alpha) {

	if (part != NULL) {
        for(int i = 0; i < total_actors; i++) {
            PRINTER() printf("neighbors of vertex %ld :\t", i);
            for(int j = xadj[i]; j < xadj[i + 1]; j++) {
                PRINTER() printf("<%ld, %ld> ", adjncy[j], (*adjwgt)[j]);
                if(i != adjncy[j] && part[i] == part[adjncy[j]]) {
                    if ((*adjwgt)[i] == 0 && (*adjwgt)[j] == 0) { //no communication
						PRINTER()
						printf(
						    "DISCONNECTED vertex %d and vertex %d are on the same partition (%d, %d) ! Reward them -> (%d, %d) [alpha %ld] \n",
						    i, adjncy[j], part[i], part[adjncy[j]], (*adjwgt)[i], (*adjwgt)[j], alpha);
						(*adjwgt)[i] = round(max((*adjwgt)[i] - alpha / (1 + (*adjwgt)[i]), (*adjwgt)[i]));
						(*adjwgt)[j] = round(max((*adjwgt)[j] - alpha / (1 + (*adjwgt)[j]), (*adjwgt)[j]));
						PRINTER()
						printf("After update → adjwgt[i] %ld \t adjwgt[neighbor] %ld \t vwgt[i] %ld \t vwgt[neghbor] %ld \n",
									(*adjwgt)[i], (*adjwgt)[j], (*vwgt)[i], (*vwgt)[adjncy[j]] );
					}else {
						PRINTER()
						printf(
						    "CHAIN vertex %d and vertex %d are on the same partition (%d, %d) ! Reward them -> (%d, %d) [alpha %ld] \n",
						    i, adjncy[j], part[i], part[adjncy[j]], (*adjwgt)[i], (*adjwgt)[adjncy[j]], alpha);
						(*adjwgt)[i] = round(max((*adjwgt)[i] + alpha, (*adjwgt)[i]));
						(*adjwgt)[j] = round(max((*adjwgt)[j] + alpha, (*adjwgt)[j]));
						PRINTER()
						printf("After update → adjwgt[i] %ld \t adjwgt[neighbor] %ld \t vwgt[i] %ld \t vwgt[neghbor] %ld \n",
									(*adjwgt)[i], (*adjwgt)[j], (*vwgt)[i], (*vwgt)[adjncy[j]] );
					}

					(*vwgt)[i] = min(round((*vwgt)[i] * 1.05) , (*vwgt)[i] + 1);
					(*vwgt)[adjncy[j]] = min(round((*vwgt)[adjncy[j]] * 1.05)  , (*vwgt)[adjncy[j]] + 1);
                }
            }
            PRINTER() printf("\n");
        }
    }


}

void metis_partitioning(idx_t total_actors, idx_t n_cus, idx_t *tasks_forecast, idx_t *capacity,
    real_t input_comm_cost_matrix[actors][actors], real_t input_anno_matrix[actors][actors], real_t input_msg_exch_cost[n_cus][n_cus])
{
	idx_t nParts = n_cus; // Number of partitions (number of CUs)

	idx_t ncon = 1; // number of weights associated to each vertex

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


	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, &vwgt, NULL, anno_matrix, 0);


	update_weights(total_actors, part_o, xadj, adjncy, &adjwgt, &vwgt, alpha);
	

	PRINTER() printf("(first) avg edges %ld \t avg vertexes %ld\n", avg_edge_wgt, avg_vert_wgt);

	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


	PRINTER() printf("total_actors %ld \t cus %ld \n", total_actors, n_cus);


	PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE **** \n");
	compute_partition(total_actors, xadj, adjncy, NULL, NULL, adjwgt, nParts, NULL, NULL, ubfactor, &alpha, &part_a, 0);

	free(xadj);
	free(adjncy);
	free(adjwgt);

	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, &vwgt, comm_cost_matrix, NULL, 1);

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

	metis_init(total_actors, n_cus, &xadj, &adjncy, &adjwgt, &vwgt, comm_cost_matrix, NULL, 0);

	PRINTER() printf("(third) avg edges %ld \t avg vertexes %ld\n", avg_edge_wgt, avg_vert_wgt);	

	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


	update_weights(total_actors, part_c, xadj, adjncy, &adjwgt, &vwgt, alpha);


	ubfactor = 100;

	PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);

    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE AND COMMUNICATION COST AND OVERLOAD **** \n");
    compute_partition(total_actors, xadj, adjncy, vwgt, NULL, adjwgt, nParts, NULL, NULL, ubfactor, &alpha, &part_o, 0);


	for(int k = 0; k < total_actors; k++) {
		if(capacity[part_o[k]] > 0) {
			capacity[part_o[k]]--;
		} else {
			for(int j = 0; j < n_cus; j++) {
				if(capacity[j] > 0) {
					part_o[k] = j;
					capacity[j]--;
				}
			}
		}
	}


	PRINTER() print_partition(total_actors, part_o);


	free(xadj);
	free(adjncy);
	free(adjwgt);
	free(vwgt);

	free(new_xadj);
	free(new_adjncy);
	free(new_vwgt);
	free(new_adjwgt);
}

idx_t *metis_get_partitioning(void)
{
	PRINTER() printf("*** PARTITION TO BE INSTALLED **** \n");
	return part_o;
}
