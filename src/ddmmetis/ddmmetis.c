#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <metis.h>
#include "utils.h"

idx_t nEdges;
idx_t actors;
idx_t cus;

idx_t *part_o;
idx_t *part_a;
idx_t *part_c;

real_t comm_cost_matrix[NUM_ACTORS][NUM_ACTORS];
real_t anno_matrix[NUM_ACTORS][NUM_ACTORS];

void ddmmetis_init(idx_t total_actors, idx_t total_cus, real_t input_comm_cost_matrix[actors][actors], real_t input_anno_matrix[actors][actors])
{

    actors = total_actors;
    cus = total_cus;


    if (input_comm_cost_matrix != NULL) memcpy(comm_cost_matrix, input_comm_cost_matrix, sizeof(comm_cost_matrix));
    if (input_anno_matrix != NULL) memcpy(anno_matrix, input_anno_matrix, sizeof(anno_matrix));
}


void metis_init(idx_t actors, idx_t cus, idx_t **xadj, idx_t **adjncy, idx_t **adjwgt, idx_t **vwgt, real_t comm_cost_matrix[actors][actors], real_t anno_matrix[actors][actors], int aug)
{


    Edge *edges = malloc(sizeof(Edge)*actors*actors);
    edges = createEdges(actors, comm_cost_matrix, anno_matrix, &nEdges);

    idx_t total_actors = (aug) ? actors * cus : actors;
    generateCSR(total_actors, nEdges, edges, xadj, adjncy, adjwgt);

    free(edges);

}

void metis_partitioning(idx_t total_actors, idx_t cus, idx_t *tasks_forecast, idx_t *capacity, real_t input_comm_cost_matrix[actors][actors], real_t input_anno_matrix[actors][actors])
{

    idx_t nParts = cus;         // Number of partitions (number of CUs)
    idx_t edgeCut;                     // Output: edge cut
    idx_t options[METIS_NOPTIONS];     // Options array
    idx_t *vwgt = tasks_forecast; // Vertex weights (used by metis to balance the partitions)

    idx_t ncon = 1;                   // number of weights associated to each vertex

    idx_t ubfactor = 30; //default imbalance tolerance
    // real_t tpwgts[cus] = {0.6, 0.3, 0.1};  // sum of elements must be 1

    idx_t *xadj, *adjncy, *adjwgt;

    if (input_comm_cost_matrix != NULL) memcpy(comm_cost_matrix, input_comm_cost_matrix, sizeof(comm_cost_matrix));
    if (input_anno_matrix != NULL) memcpy(anno_matrix, input_anno_matrix, sizeof(anno_matrix));


    metis_init(total_actors, cus, &xadj, &adjncy, &adjwgt, &vwgt, NULL, anno_matrix, 0);

    PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


    double lambda = 0.2;
    idx_t iteration = 1;

    PRINTER() printf("total_actors %ld \t cus %ld \n", total_actors, cus);


    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE **** \n");
    compute_partition(total_actors, xadj, adjncy, vwgt, NULL, adjwgt, nParts, NULL, NULL, edgeCut, ubfactor, &part_a, 0);


    metis_init(total_actors, cus, &xadj, &adjncy, &adjwgt, &vwgt, comm_cost_matrix, NULL, 1);



    for (int i=0; i < total_actors; i++) {
        PRINTER() printf("neighbors of vertex %ld :\t", i);
        for (int j = xadj[i]; j < xadj[i+1]; j++) {
            PRINTER() printf("<%ld, %ld> ",adjncy[j], adjwgt[j]);
            if (i != adjncy[j] && part_a[i] == part_a[adjncy[j]]) {
                PRINTER() printf("vertex %ld and vertex %ld are on the same partition (%ld, %ld) ! Reward them -> (%ld, %ld)\n", i, adjncy[j] , part_a[i], part_a[adjncy[j]], adjwgt[i], adjwgt[adjncy[j]]);
                adjwgt[i] = fmax(adjwgt[i]*1000, 1);
                adjwgt[adjncy[j]] = fmax(adjwgt[adjncy[j]]*1000,1);
                vwgt[i] = fmax(vwgt[i]*10, 1);
                vwgt[adjncy[j]] = fmax(vwgt[adjncy[j]]*10,1);
	    }
	}
	PRINTER() printf("\n");
    }

    PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


    idx_t aug_v = total_actors*cus;
    idx_t *new_adjwgt;
    idx_t *new_vwgt;

    idx_t *new_xadj = populate_newxadj(aug_v/cus, cus, xadj, vwgt, &new_vwgt);

    idx_t maxEdges = new_xadj[aug_v];

    PRINTER() printf("NVERTICES %ld \t MAXEDGES %ld\n", aug_v, maxEdges);

    idx_t *new_adjncy = populate_newadjncy(aug_v/cus, cus, maxEdges, xadj, new_xadj, adjncy, adjwgt, &new_adjwgt);

    PRINTER() print_csr_graph(aug_v, new_xadj, new_adjncy, new_vwgt, new_adjwgt);

    ubfactor = 50;

    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE AND COMMUNICATION COST **** \n");
    compute_partition(aug_v, new_xadj, new_adjncy, new_vwgt, NULL, new_adjwgt, nParts, NULL, NULL, edgeCut, ubfactor, &part_c, 1);

    iteration++;


    metis_init(total_actors, cus, &xadj, &adjncy, &adjwgt, &vwgt, comm_cost_matrix, NULL, 0);


    PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


    for (int i=0; i < total_actors; i++) {
        PRINTER() printf("neighbors of vertex %ld :\t", i);
        for (int j = xadj[i]; j < xadj[i+1]; j++) {
            PRINTER() printf("<%ld, %ld> ",adjncy[j], adjwgt[j]);
            if (i != adjncy[j] && part_c[i] == part_c[adjncy[j]]) {
                PRINTER() printf("vertex %ld and vertex %ld are on the same partition (%ld, %ld) ! Reward them -> (%ld, %ld)\n", i, adjncy[j] , part_c[i], part_c[adjncy[j]], adjwgt[i], adjwgt[adjncy[j]]);
                adjwgt[i] = fmax(adjwgt[i]*1000, 1); //(scale_weight(adjwgt[i] *exp(-lambda*iteration), SCALE), 1);
		adjwgt[adjncy[j]] = fmax(adjwgt[adjncy[j]] * 1000, 1);
		vwgt[i] = fmax(vwgt[i]*100, 1);
                vwgt[adjncy[j]] = fmax(vwgt[adjncy[j]]*100,1);
            }
        }
        PRINTER() printf("\n");
    }

    ubfactor = 100;

    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE AND COMMUNICATION COST AND OVERLOAD **** \n");
    compute_partition(total_actors, xadj, adjncy, vwgt, NULL, adjwgt, nParts, NULL, NULL, edgeCut, ubfactor, &part_o, 0);


    for (int i=0; i < total_actors; i++) {
        PRINTER() printf("neighbors of vertex %ld :\t", i);
        for (int j = xadj[i]; j < xadj[i+1]; j++) {
            PRINTER() printf("<%ld, %ld> ",adjncy[j], adjwgt[j]);
            if (i != adjncy[j] && part_o[i] == part_o[adjncy[j]]) {
                PRINTER() printf("vertex %ld and vertex %ld are on the same partition (%ld, %ld) ! Reward them -> (%ld, %ld)\n", i, adjncy[j] , part_o[i], part_o[adjncy[j]], adjwgt[i], adjwgt[adjncy[j]]);
                adjwgt[i] = fmax(adjwgt[i]*1000, adjwgt[i]);
                adjwgt[adjncy[j]] = fmax(adjwgt[adjncy[j]]*1000,adjwgt[adjncy[j]]);
                vwgt[i] = fmax(vwgt[i]*10, 1);
                vwgt[adjncy[j]] = fmax(vwgt[adjncy[j]]*10,1);
	    }
	}
	PRINTER() printf("\n");
    }

    for ( int k = 0; k < total_actors; k++){
        if (capacity[part_o[k]] > 0) {
            capacity[part_o[k]]--;
        } else {
            for (int j = 0; j < cus; j++) {
                if (capacity[j] > 0) {
                    part_o[k] = j;
                    capacity[j]--;
                }
            }
        }
    }


    PRINTER() print_partition(total_actors, part_o);



    free(new_xadj);
    free(new_adjncy);
    free(new_vwgt);
    free(new_adjwgt);

}

idx_t * metis_get_partitioning() {

    PRINTER() printf("*** PARTITION TO BE INSTALLED **** \n");
    return part_o;
}
