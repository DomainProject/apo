#include <metis.h>

// cu
enum cu_type { CPU = 1, GPU = 2, FPGA = 4 };


extern void ddmmetis_init(idx_t actors);
extern void metis_heterogeneous_multilevel(idx_t total_actors, idx_t cus, idx_t *tasks_forecast, idx_t *capacity, real_t input_comm_cost_matrix[total_actors][total_actors], real_t input_anno_matrix[total_actors][total_actors], real_t input_msg_exch_cost[n_cus][n_cus]);
extern void metis_communication(idx_t total_actors, idx_t cus, idx_t *tasks_forecast,  real_t input_comm_cost_matrix[total_actors][total_actors], real_t input_msg_exch_cost[n_cus][n_cus]);
extern void metis_baseline(idx_t total_actors, idx_t n_cus, idx_t *tasks_forecast, real_t input_comm_cost_matrix[actors][actors], real_t input_tpwgts[n_cus]);
extern idx_t* metis_get_partitioning(void);
