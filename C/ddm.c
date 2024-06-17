#include <stdlib.h>
#include <assert.h>
#include "ddm.h"

static int num_cus;
static enum cu_type *mapping_cus;

void ddm_init(int total_cus, int total_actors, enum cu_type *cus, int msg_exch_cost[total_cus][total_cus], short runnable_on[total_actors])
{
    num_cus = total_cus;
    mapping_cus = cus;
}

// Restituisce un vettore in cui nella posizione i-esima è conservato il dispositivo su cui deve girare l'attore i-esimo
enum cu_type *ddm_optimize(int total_actors, struct actor_matrix actors[total_actors][total_actors], int tasks_forecast[total_actors])
{
    return NULL;
}
