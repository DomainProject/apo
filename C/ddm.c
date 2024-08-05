#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ddm.h"

static int num_cus;
static enum cu_type *mapping_cus;
static int *cu_capacity; 

void ddm_init(
    int total_cus,
    int total_actors,
    enum cu_type *cus,
    int msg_exch_cost[total_cus][total_cus],
    short runnable_on[total_actors],
    int cu_capacity[total_cus])
{
    num_cus = total_cus;
    mapping_cus = cus;

    // actor/1
    printf("actor(1..%d).\n", total_actors);

    // runnable_on/2
    for(int i=0; i<total_actors; ++i) {
      if (runnable_on[i] & 1)
        printf("runnable_on(%d,cpu).\n",i);
      if (runnable_on[i] & (1 << 2))
        printf("runnable_on(%d,gpu).\n",i);
      if (runnable_on[i] & (1 << 3))
        printf("runnable_on(%d,fpga).\n",i);
    }
   
    // cu/1
    printf("cu(1..%d).\n", total_cus);

    
    for(int i=0; i<total_cus; ++i) {
      // cu_type/2
      switch(cus[i]) {
        case CPU:
          printf("cu_type(%d,cpu).\n",i);
          break;
        case GPU:
          printf("cu_type(%d,gpu).\n",i);
          break;
        case FPGA:  
          printf("cu_type(%d,fpga).\n",i);
          break;
      }
      // cu_capacity/2 
      printf("cu_capacity(%d).\n",cu_capacity[i]);
    }

    // msg_exch_cost/3
    for(int i=0; i<total_cus; ++i)
      for(int j=0; j<total_cus; ++j)
        printf("msg_exch_cost(%d,%d,%d).\n",i,j,msg_exch_cost[i][j]);

}

// Restituisce un vettore in cui nella posizione i-esima è conservato il dispositivo su cui deve girare l'attore i-esimo
enum cu_type *ddm_optimize(
    int total_actors,
    struct actor_matrix actors[total_actors][total_actors],
    int tasks_forecast[total_actors])
{
    // tasks_forecast/2
    for(int i=0; i<total_actors; ++i) {
      printf("tasks_forecast(%d,%d).\n",i,tasks_forecast[i]);
    }    

    for(int i=0; i<total_actors; ++i) {
      for(int j=0; j<total_actors; ++j) {
        // msg_exchange_rate/3
        if(actors[i][j].msg_exchange_rate)
          printf("msg_exchange_rate(%d,%d,%d).\n",i,j,
            actors[i][j].msg_exchange_rate);
      }
    } 

    for(int i=0; i<total_actors; ++i) {
      for(int j=0; j<total_actors; ++j) {
        // mutual_annoyance/3
        if(actors[i][j].annoyance)
          printf("mutual_annoyance(%d,%d,%d).\n",i,j,
            actors[i][j].annoyance);
      }
    }     

    // msg_exch_rate/3

    // TODO: add get_model to libasp_solver
    // that returns a clingo model (not an array of strings)
   //char *atom, *snd, *end;
    //for(int i=0; i<total_actors; ++i) {
    //  atom +=7; // skip run_on(
    //  printf("%ld %ld\n", strtol(atom, &snd, 10), strtol(++snd, &end, 10));   
   // }

    return NULL;
}
