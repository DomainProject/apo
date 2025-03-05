#pragma once

#include <metis.h>
#include <float.h>


#define N_CU_TYPES 3
#define N_CUS 4
#define NUM_ACTORS 10
#define MAX_WEIGHT 10000L
#define SCALE 1.0

#define PRINTER() if (1)

//extern idx_t actors;
extern idx_t avg_edge_wgt;
extern idx_t avg_vert_wgt;
//extern real_t comm_cost_matrix[NUM_ACTORS][NUM_ACTORS];
//extern real_t anno_matrix[NUM_ACTORS][NUM_ACTORS];

// Define the structure
typedef struct Edge {
    idx_t src;    // Source vertex
    idx_t dest;   // Destination vertex
    idx_t weight; // Edge weight (METIS does not support real_t as weights)
    idx_t annoy;  // Another edge weight -- annoyance
} Edge;

void generateCSR(idx_t nVertices, idx_t nEdges, Edge *edges, idx_t **xadj, idx_t **adjncy, idx_t **adjwgt);
Edge * createEdges(idx_t actors, real_t comm_matrix[actors][actors], real_t anno_matrix[actors][actors], idx_t *edge_count);
idx_t scale_weight(real_t real_weight, float scale_factor);

int is_compatible(int actor, int cu);
void print_cardinality(void);

void validate_partition(idx_t nVertices, idx_t *part);
void print_partition(idx_t nVertices, idx_t *part);
void print_csr_graph(idx_t nVertices, idx_t *xadj, idx_t *adjncy, idx_t *vwgt, idx_t *adjwgt);
idx_t * remap_partitioning(idx_t nVertices, idx_t cus, idx_t *part);

idx_t *populate_newxadj(idx_t nVertices, idx_t cus, idx_t *xadj, idx_t *vwgt, idx_t **new_vwgt);
idx_t *populate_newadjncy(idx_t nVertices, idx_t cus, idx_t maxEdges, idx_t *xadj, idx_t *new_xadj, idx_t *adjncy, idx_t *adjwgt, idx_t **new_adjwgt, idx_t msg_exch_cost[cus][cus]);


void compute_partition(idx_t nVertices, idx_t *xadj, idx_t *adjncy, idx_t *vwgt, idx_t *vsize, idx_t *adjwgt,
    idx_t nParts, real_t *tpwgts, real_t *ubvec, idx_t ubfactor, idx_t *alpha, idx_t **partition, int remap);
