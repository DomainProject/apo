#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ddm.h"

static int num_cus;
static enum cu_type *mapping_cus;

void ddm_init(
    int total_cus,
    int total_actors,
    enum cu_type *cus,
    int msg_exch_cost[total_cus][total_cus],
    short runnable_on [total_actors])
{
    num_cus = total_cus;
    mapping_cus = cus;

    // actor/1
    printf("actor(1..%d).\n", total_actors);

    // runnable_on/2
    // *** TODO: combinazioni?
    for(int i=0; i<total_actors; ++i)
      switch(runnable_on[i]) {
        case 1: 
          printf("runnable_on(%d,cpu)",i);
          break;
        case 2: 
          printf("runnable_on(%d,gpu)",i);
          break;
        case 4: 
          printf("runnable_on(%d,fpga)",i);
          break;
        // combinazioni
        default: 
         printf("???"); break;
      }

   
    // cu/1
    printf("cu(1..%d).\n", total_cus);

    // cu_type/2
    // *** TODO: non rappresentato

    // cu_capacity/2 
    // *** TODO: non rappresentato

    // msg_exch_cost/3
    // TODO: vedi definifizione in dd_v4_init.asp
    // msg_exch_cost(cpu,cpu,1). % c'è il tipo, dobbiamo usare l'identificatiore della specifica CU?
    // cambiamo la definizione di a_cc(A1,A2,C)? invece del tipo usiamo l'identificatore della CU
    for(int i=0; i<total_cus; ++i)
      for(int j=0; i<total_cus; ++j)
      printf("msg_exch_cost(%d,%d,%d)",i,j,msg_exch_cost[i][j]);

}

// Restituisce un vettore in cui nella posizione i-esima è conservato il dispositivo su cui deve girare l'attore i-esimo
enum cu_type *ddm_optimize(
    int total_actors,
    struct actor_matrix actors[total_actors][total_actors],
    int tasks_forecast[total_actors])
{
    // tasks_forecast/2

    // mutual_annoyance/3

    // msg_exch_rate/3

    // see clingo_model_to_strings
    char *atom, *snd, *end;
    for(int i=0; i<total_actors; ++i) {
      atom +=7; // skip run_on(
      printf("%ld %ld\n", strtol(atom, &snd, 10), strtol(++snd, &end, 10));   
    }

    return NULL;
}
