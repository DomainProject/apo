#include <metis.h>

// cu
enum cu_type { CPU = 1, GPU = 2, FPGA = 4 };


extern void ddmmetis_init(idx_t actors, idx_t cus, real_t **input_comm_cost_matrix, real_t **input_anno_matrix);
extern void metis_partitioning(idx_t total_actors, idx_t cus, idx_t *tasks_forecast, idx_t *capacity, real_t input_comm_cost_matrix[actors][actors], real_t input_anno_matrix[actors][actors])
extern idx_t* metis_get_partitioning(void);

