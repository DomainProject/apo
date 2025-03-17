#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "utils.h"


/**
 *  DEBUG FUNCTION
 * */

void print_csr_graph(idx_t nVertices, idx_t *xadj, idx_t *adjncy, idx_t *vwgt, idx_t *adjwgt) {


    printf("xadj: ");
    for (int i = 0; i <= nVertices; i++) {
        printf("%ld ", xadj[i]);
    }
    printf("\n");

    printf("adjncy: ");
    for (int i = 0; i < xadj[nVertices]; i++) {
        printf("%ld ", adjncy[i]);
    }
    printf("\n");

    printf("adjwgt: ");
    for (int i = 0; i < xadj[nVertices]; i++) {
        printf("%ld ", adjwgt[i]);
    }
    printf("\n");

    printf("vwgt: ");
    for (int i = 0; i < nVertices; i++) {
        printf("%ld ", vwgt[i]);
    }
    printf("\n");
}

/**
 * POST PROCESSING FUNCTIONS
 * */



void print_partition(idx_t nVertices, idx_t *part) {



    printf("\n=== Partition === \n");

    // Print the actors and the CUs assigned to them for the current partition
    for (long actor = 0; actor < nVertices; actor++) {
        int cu = part[actor];
        if (cu != -1) {
            printf("Actor %ld -> CU %d\n", actor, cu);
        }
    }

}




idx_t * remap_partitioning(idx_t nVertices, idx_t cus, idx_t *part) {


    idx_t *newpart = malloc(nVertices/cus * sizeof(idx_t));
    if (!newpart) {
        fprintf(stderr, "Error in allocating newpart \n");
        exit(1);
    }
    PRINTER() printf("\n=== Remap Partition ===\n");

    // Print the actors and the CUs assigned to them for the current partition
    for (int actor = 0; actor < nVertices; actor+=cus) {
        int cu = part[actor];
        if (cu != -1) {
            //printf("Actor %d -> CU %d\n", actor/N_CU_TYPES, cu);
            newpart[actor/cus] = cu;
        }
    }
    return newpart;

}

/**
 *  INIT FUNCTIONS
 * */

idx_t scale_weight(real_t real_weight, float scale_factor) {
    real_t scaled_value = real_weight * scale_factor;
    if (scaled_value > REAL_MAX) {
        scaled_value = REAL_MAX;  // Ensure it doesn't overflow
    }
    return (idx_t)(scaled_value);  // Ensure we cast it after scaling
}


Edge * createEdges(idx_t actors, real_t comm_cost_matrix[actors][actors],
                 real_t anno_matrix[actors][actors], idx_t *edge_count) {

    Edge *edges = malloc(sizeof(Edge)*actors*actors);
    // Determine which matrix to use
    real_t (*matrix)[actors] = (comm_cost_matrix == NULL) ? anno_matrix : comm_cost_matrix;

    int only_self_edges = 1;
    for (int i=0; i < actors; i++) {
        for (int j = i+1; j < actors; j++) {
            if ((matrix)[i][j] > 0) {
                only_self_edges = 0;
                break;
            }
        }
    }

    int k = 0;
    for (int i = 0; i < actors; i++) {
        for (int j = i+1; j < actors; j++) {
            if ((matrix)[i][j] > 0.0 && i != j) {  // Only consider valid edges but consider self-edges
                //printf("Original value at [%d][%d] = %.6f\n", i, j, (*matrix)[i][j]);
                edges[k].src = i;
                edges[k].dest = j;
                real_t avg = fmax(((matrix)[i][j] + (matrix)[j][i] + 1.0f) / 2.0f, 0.0f);
                //real_t avg = ((*matrix)[i][j] + (*matrix)[j][i] / 2.0f > 1) ? ((*matrix)[i][j] + (*matrix)[j][i]) / 2.0f : 1.0;
                edges[k].weight = scale_weight(avg, SCALE);

                PRINTER() printf("edges SRC %ld \t DEST %ld WEIGHT %ld (from comm_matrix[i][j] %.6f)\n", edges[k].src, edges[k].dest, edges[k].weight, (matrix)[i][j]);
                k++;
            }
        }
    }

    if (only_self_edges) {
        for (int i = 0; i < k; i++) {
            edges[k].src = 0;
            edges[k].dest = 0;
            edges[i].weight = 0;
        }
    }
    *edge_count = k;  // Store the total number of edges

    return edges;
}





/// Function to generate CSR representation
void generateCSR(idx_t nVertices, idx_t nEdges, Edge *edges, idx_t **xadj, idx_t **adjncy, idx_t **adjwgt) {
    /// Allocate memory for CSR arrays
    *xadj = calloc((nVertices + 1), sizeof(idx_t));
    *adjncy = calloc(2 * nEdges, sizeof(idx_t)); // 2*nEdges for undirected graphs
    *adjwgt = calloc(2 * nEdges , sizeof(idx_t));

    if (!*xadj || !*adjncy || !*adjwgt) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }


    /// Initialize xadj to 0
    for (int i = 0; i <= nVertices; i++) (*xadj)[i] = 0;

    int *degree = calloc(nVertices, sizeof(idx_t));
    // Count degrees for each vertex
    for (int i = 0; i < nEdges; i++) {

        idx_t src = edges[i].src;
        idx_t dest = edges[i].dest;
        PRINTER() printf(" src  %ld \t dest %ld \n", edges[i].src , edges[i].dest);
        PRINTER() printf("(*xadj)[edges[i].src  %ld \t (*xadj)[edges[i].dest %ld \n", (*xadj)[edges[i].src] + 1 , (*xadj)[edges[i].dest] + 1);
        (*xadj)[src]++;
        (*xadj)[dest]++; // For undirected graph
        degree[src]++;
        degree[dest]++;
    }

    PRINTER() for(int i=0; i < nVertices; i++) printf("degree %d\n", degree[i]);


    (*xadj)[0] = 0;
    for (int i = 1; i <= nVertices; i++) {
        (*xadj)[i] = degree[i-1] + (*xadj)[i-1]; // Set prefix sum
    }


    // Temporary array to track insertion position
    idx_t *temp = malloc(nVertices * sizeof(idx_t));
    memcpy(temp, *xadj, (nVertices + 1) * sizeof(idx_t));
    for (int i = 0; i <= nVertices; i++) {
        temp[i] = (*xadj)[i];
    }


    // Fill adjncy array
    for (int i = 0; i < nEdges; i++) {
        idx_t src = edges[i].src;
        idx_t dest = edges[i].dest;
        idx_t w = edges[i].weight;

        (*adjwgt)[temp[src]] = w;
        (*adjwgt)[temp[dest]] = w;

        (*adjncy)[temp[src]++] = dest;
        (*adjncy)[temp[dest]++] = src; // For undirected graph

        /*(*adjwgt)[temp[src]] = w;
        (*adjncy)[temp[src]] = dest;
        temp[src]++;

        (*adjwgt)[temp[dest]] = w;
        (*adjncy)[temp[dest]] = src;
        temp[dest]++;*/

        PRINTER() printf("src %ld : weight source %ld --- dest %ld : weight dest %ld\n", src, (*adjwgt)[(*xadj)[src]],dest, (*adjwgt)[(*xadj)[dest]]);
        PRINTER() printf("weight edges %ld -- %ld -- %ld \n", w, temp[src], temp[dest]);
    }

    free(degree);
    free(temp);
}


idx_t *populate_newxadj(idx_t nVertices, idx_t cus, idx_t *xadj, idx_t *vwgt, idx_t **new_vwgt) {

    idx_t *new_xadj = (idx_t *)calloc((nVertices*cus + 1), sizeof(idx_t));
    if (!new_xadj) {
        fprintf(stderr, "Allocation error for new_xadj \n");
        exit(1);
    }

    *new_vwgt = (idx_t *)calloc((nVertices*cus + 1), sizeof(idx_t));
    if (!*new_vwgt) {
        fprintf(stderr, "Allocation error for new_vwgt \n");
        exit(1);
    }

    new_xadj[0] = 0;
    (*new_vwgt)[0] = vwgt[0];

    for (int i = 0; i < nVertices; i++) {
        for (int j = 0; j < cus; j++) {
            new_xadj[i*cus + j + 1] = (xadj[i+1] - xadj[i])*cus+(cus-1) + new_xadj[i*cus + j];
            (*new_vwgt)[i*cus + j + 1] = vwgt[i];
            PRINTER() printf("[populate_newxadj] i %d \t j %d \t value %ld\n", i, j, (xadj[i+1] - xadj[i])*cus+(cus-1) + new_xadj[i*cus + j]);

        }
    }

    return new_xadj;
}


idx_t *populate_newadjncy(idx_t nVertices, idx_t cus, idx_t maxEdges, idx_t *xadj, idx_t *new_xadj, idx_t *adjncy, idx_t *adjwgt, idx_t **new_adjwgt, idx_t msg_exch_cost[cus][cus]) {

    idx_t *new_adjncy = (idx_t *) calloc(maxEdges, sizeof(idx_t));
    if (!new_adjncy) {
        fprintf(stderr, "Allocation error for new_adjncy \n");
        exit(1);
    }


    *new_adjwgt = (idx_t *)calloc(maxEdges, sizeof(idx_t));
    if (!*new_adjwgt) {
        fprintf(stderr, "Allocation error for new_adjwgt \n");
        exit(1);
    }

    idx_t index = 0;

    for (int i = 0; i < nVertices; i++) {
        idx_t s = xadj[i];
        for (idx_t j = s; j < xadj[i+1]; j++) {
            for (int h=0; h < cus; h++) {
                for (int k = 0; k < cus; k++) {

                        index = new_xadj[i*cus+h] + (j-s)*cus + k;
                        if (index >= maxEdges || index < 0) {
                            fprintf(stderr, "Error: Index %ld out of bounds (maxEdges = %ld)\n", index, maxEdges);
                            exit(EXIT_FAILURE);
                     }
                        new_adjncy[index] = adjncy[j]*cus+k;
                        PRINTER() printf("[populate_newadjncy] i %d \t j %ld \t (h,k) (%d, %d) \t new_xadj[i*cus+h] %ld \t (j-s)*cus + k %ld \t index %ld \t value %ld \t adjwgt[j] * msg_exch_cost[h][k] %ld \n", i, j, h, k, new_xadj[i*cus+h], (j-s)*cus + k, index, adjncy[j]*cus+k, adjwgt[j] * msg_exch_cost[h][k]);
                        (*new_adjwgt)[index] = adjwgt[j] * msg_exch_cost[h][k];

                } ///end for k
                for (int l = 0; l < h; l++) {
                    PRINTER() printf("l + index %ld \t l %ld\n", l+index+1, l+i*cus);
                    new_adjncy[l+index+1] = l+i*cus;
                    (*new_adjwgt)[l+index+1] = MAX_WEIGHT;
                }
                for (int x = h+1; x < cus; x++) {
                    PRINTER() printf("x + index %ld \t x %ld\n", x+index, x+i*cus);
                    new_adjncy[x+index] = x+i*cus;
                    (*new_adjwgt)[x+index] = MAX_WEIGHT;
                }
            } ///end for h
        } ///end for j
    } ///end for i

    return new_adjncy;
}



/*** int METIS PartGraphKway(idx t *nvtxs, idx t *ncon, idx t *xadj, idx t *adjncy,
idx t *vwgt, idx t *vsize, idx t *adjwgt, idx t *nparts, real t *tpwgts,
real t ubvec, idx t *options, idx t *objval, idx t *part)
 *
 * @param nvtxs: number of vertices
 * @param ncon: number of balancing constraints (weights associated to each vertex) → at least 1
 * @param xadj: indexes of the adjacency list
 * @param adjncy: adjaceny list in csr format
 * @param vwgt: weights of the vertices (NULL if not)
 * @param vsize: size of the vertices (NULL if 1)
 * @param adjwgt: weights of the edges (NULL if not)
 * @param nparts: number of partitions
 * @param tpwgts: array of size nparts×ncon that specifies the desired weight for each partition and constraint
 *                for each constraint, the sum of the tpwgts[] entries must be 1.0
 *                (NULL if the graph must be partitioned equally, so partitions do not have "priorities")
 * @param ubvec: array of size ncon that specifies the allowed load imbalance tolerance for each constraint
 *               the load imbalances must be greater than 1.0
 *               (NULL if load imbalance tolerance is 1.0001)
 * @param options: array of options METIS_OPTION_OBJTYPE, METIS_OPTION_CTYPE, METIS_OPTION_IPTYPE,
                                    METIS_OPTION_RTYPE, METIS_OPTION_NCUTS, METIS_OPTION_NITER,
                                    METIS_OPTION_UFACTOR, METIS_OPTION_MINCONN, METIS_OPTION_CONTIG,
                                    METIS_OPTION_SEED, METIS_OPTION_NUMBERING, METIS_OPTION_DBGLVL
 * @param objval: Upon successful completion, this variable stores the edge-cut or the total communication volume of
                  the partitioning solution
 * @param part: vector of size nvtxs that upon successful completion stores the partition vector of the graph
 *
 *
 * @return: METIS_OK upon success
 *          METIS_ERROR_INPUT upon input error
 *          METIS_ERROR_MEMORY upon error on memory allocation
 *          METIS_ERROR upon general error
 *
 * */

void compute_partition(idx_t nVertices, idx_t *xadj, idx_t *adjncy, idx_t *vwgt, idx_t *vsize, idx_t *adjwgt,
    idx_t nParts, real_t *tpwgts, real_t *ubvec, idx_t ubfactor, idx_t *alpha, idx_t **partition, int remap)
{

    idx_t ncon = 1;
    idx_t edgeCut;                    // Output: edge cut of the partitioning

    *partition = malloc(sizeof(idx_t) * nVertices);
    idx_t options[METIS_NOPTIONS];     // Options array

    METIS_SetDefaultOptions(options); // Initialize default options
    options[METIS_OPTION_NUMBERING] = 0; // C-style numbering (0-based)
    options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY; //Multilevel k-way partitioning
    options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_VOL; //Minimize total communication volume
    options[METIS_OPTION_NCUTS] = 3; // Specifies the number of different partitionings that it will compute. The final partitioning is the one that
                                     //achieves the best edgecut or communication volume. Default is 1.

    options[METIS_OPTION_UFACTOR] = ubfactor; //10% imbalance for connectivity preservation

    // Perform partitioning
    int status = METIS_PartGraphKway(
        &nVertices, &ncon, xadj, adjncy, vwgt, vsize, adjwgt,
        &nParts, tpwgts, ubvec, options, &edgeCut, *partition
    );

    if (status == METIS_OK) {

        *alpha = (edgeCut == 0) ? 1 : xadj[nVertices] / (edgeCut + 1);

        PRINTER() printf("EDGE CUT %ld \t total edges %ld \t ratio %ld  n", edgeCut, xadj[nVertices], (idx_t) (xadj[nVertices]/edgeCut));
        if (!remap) {
            PRINTER() print_partition(nVertices, *partition);
        } else {

            *partition = remap_partitioning(nVertices, nParts, *partition);
            PRINTER() print_partition(nVertices/nParts, *partition);
        }
    } else {
        fprintf(stderr, "METIS partitioning failed with error %u.\n", status);
    }
}
