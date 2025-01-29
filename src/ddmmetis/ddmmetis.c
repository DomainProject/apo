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

idx_t avg_edge_wgt;
idx_t avg_vert_wgt;

idx_t *part_o;
idx_t *part_a;
idx_t *part_c;

real_t comm_cost_matrix[NUM_ACTORS][NUM_ACTORS];
real_t anno_matrix[NUM_ACTORS][NUM_ACTORS];

void ddmmetis_init(idx_t total_actors, idx_t total_cus)
{

    actors = total_actors;
    cus = total_cus;

}


void metis_init(idx_t actors, idx_t cus, idx_t **xadj, idx_t **adjncy, idx_t **adjwgt, idx_t **vwgt, real_t comm_cost_matrix[actors][actors], real_t anno_matrix[actors][actors], int aug)
{


    Edge *edges = malloc(sizeof(Edge)*actors*actors);
    edges = createEdges(actors, comm_cost_matrix, anno_matrix, &nEdges);

    idx_t total_actors = (aug) ? actors * cus : actors;
    generateCSR(total_actors, nEdges, edges, xadj, adjncy, adjwgt);

    idx_t sum_vert_wgt = 0;
    for (int i=0; i < actors; i++) {
        sum_vert_wgt += (*vwgt)[i];
    }

    idx_t sum_edge_wgt = 0;
    for (int i=0; i < (*xadj)[actors]; i++) {
        sum_edge_wgt += (*adjwgt)[i];
    }

    PRINTER() printf("sum_edge_wgt %ld \t sum_vert_wgt %ld\n", sum_edge_wgt, sum_vert_wgt);
    avg_edge_wgt = sum_edge_wgt / (*xadj)[actors];
    avg_vert_wgt = sum_vert_wgt / total_actors;


    free(edges);

}

void metis_partitioning(idx_t total_actors, idx_t cus, idx_t *tasks_forecast, idx_t *capacity, real_t input_comm_cost_matrix[actors][actors], real_t input_anno_matrix[actors][actors])
{

    idx_t nParts = cus;         // Number of partitions (number of CUs)
    idx_t edgeCut;                     // Output: edge cut

    idx_t ncon = 1;                   // number of weights associated to each vertex

    idx_t ubfactor = 30; //default imbalance tolerance
    // real_t tpwgts[cus] = {0.6, 0.3, 0.1};  // sum of elements must be 1

    idx_t *xadj, *adjncy, *adjwgt;
    idx_t *vwgt = malloc(sizeof(idx_t) * total_actors); // Vertex weights (used by metis to balance the partitions)

    for (int i =0; i < total_actors; i++) {
        vwgt[i] = tasks_forecast[i];
    }
    if (input_comm_cost_matrix != NULL) memcpy(comm_cost_matrix, input_comm_cost_matrix, sizeof(comm_cost_matrix));
    if (input_anno_matrix != NULL) memcpy(anno_matrix, input_anno_matrix, sizeof(anno_matrix));


    metis_init(total_actors, cus, &xadj, &adjncy, &adjwgt, &vwgt, NULL, anno_matrix, 0);

    PRINTER() printf("(first) avg edges %ld \t avg vertexes %ld\n", avg_edge_wgt, avg_vert_wgt);

    PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


    PRINTER() printf("total_actors %ld \t cus %ld \n", total_actors, cus);


    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE **** \n");
    compute_partition(total_actors, xadj, adjncy, vwgt, NULL, adjwgt, nParts, NULL, NULL, edgeCut, ubfactor, &part_a, 0);


    metis_init(total_actors, cus, &xadj, &adjncy, &adjwgt, &vwgt, comm_cost_matrix, NULL, 1);

    PRINTER() printf("(second) avg edges %ld \t avg vertexes %ld\n", avg_edge_wgt, avg_vert_wgt);


    for (int i=0; i < total_actors; i++) {
        PRINTER() printf("neighbors of vertex %ld :\t", i);
        for (int j = xadj[i]; j < xadj[i+1]; j++) {
            PRINTER() printf("<%ld, %ld> ",adjncy[j], adjwgt[j]);
            if (i != adjncy[j] && part_a[i] == part_a[adjncy[j]]) {
                PRINTER() printf("vertex %ld and vertex %ld are on the same partition (%ld, %ld) ! Reward them -> (%ld, %ld)\n", i, adjncy[j] , part_a[i], part_a[adjncy[j]], adjwgt[i], adjwgt[adjncy[j]]);
                adjwgt[i] = fmax(adjwgt[i]*avg_edge_wgt*2, adjwgt[i]);
                adjwgt[adjncy[j]] = fmax(adjwgt[adjncy[j]] * avg_edge_wgt * 2, adjwgt[adjncy[j]]);
                vwgt[i] = fmax(vwgt[i]*avg_vert_wgt*2, vwgt[i]+1);
                vwgt[adjncy[j]] = fmax(vwgt[adjncy[j]]*avg_vert_wgt*2, vwgt[adjncy[j]]+1);
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



    metis_init(total_actors, cus, &xadj, &adjncy, &adjwgt, &vwgt, comm_cost_matrix, NULL, 0);

    PRINTER() printf("(third) avg edges %ld \t avg vertexes %ld\n", avg_edge_wgt, avg_vert_wgt);


    PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);


    for (int i=0; i < total_actors; i++) {
        PRINTER() printf("neighbors of vertex %ld :\t", i);
        for (int j = xadj[i]; j < xadj[i+1]; j++) {
            PRINTER() printf("<%ld, %ld> ",adjncy[j], adjwgt[j]);
            if (i != adjncy[j] && part_c[i] == part_c[adjncy[j]]) {
                PRINTER() printf("vertex %ld and vertex %ld are on the same partition (%ld, %ld) ! Reward them -> (%ld, %ld)\n", i, adjncy[j] , part_c[i], part_c[adjncy[j]], adjwgt[i], adjwgt[adjncy[j]]);
                adjwgt[i] = fmax(adjwgt[i]*avg_edge_wgt*2, adjwgt[i]);
    		    adjwgt[adjncy[j]] = fmax(adjwgt[adjncy[j]] * avg_edge_wgt*2, adjwgt[adjncy[j]]);
    		    vwgt[i] = fmax(vwgt[i]*avg_vert_wgt*2, vwgt[i]+1);
                vwgt[adjncy[j]] = fmax(vwgt[adjncy[j]]*avg_vert_wgt*2, vwgt[adjncy[j]]+1);
            }
        }
        PRINTER() printf("\n");
    }

    ubfactor = 100;

    PRINTER() print_csr_graph(total_actors, xadj, adjncy, vwgt, adjwgt);

    PRINTER() printf("**** THIS IS THE PARTITION MINIMIZING ANNOYANCE AND COMMUNICATION COST AND OVERLOAD **** \n");
    compute_partition(total_actors, xadj, adjncy, vwgt, NULL, adjwgt, nParts, NULL, NULL, edgeCut, ubfactor, &part_o, 0);


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


    free(vwgt);
    free(new_xadj);
    free(new_adjncy);
    free(new_vwgt);
    free(new_adjwgt);

}

idx_t * metis_get_partitioning() {

    PRINTER() printf("*** PARTITION TO BE INSTALLED **** \n");
    return part_o;
}
