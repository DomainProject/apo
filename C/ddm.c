#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ddm.h"

static int num_cus;
static enum cu_type *mapping_cus;
static int *cu_capacity; 

char *prog_buff;
char *curr_pptr;
size_t prog_size_approx;

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

    prog_size_approx = total_actors * (100 * total_actors + 120) + 
                     total_cus * (50 * total_cus + 30) + 50;

    if (!(prog_buff = malloc(prog_size_approx))) {
      perror("ddm_init: could not allocate memory for prog_buff");
      exit(errno);
    }

    curr_pptr = prog_buff;
    
    // actor/1
    curr_pptr += sprintf(curr_pptr, "actor(0..%d).\n", total_actors-1);

    // runnable_on/2
    for(int i=0; i<total_actors; ++i) {
      if (runnable_on[i] & 1)
        curr_pptr += sprintf(curr_pptr, "runnable_on(%d,cpu).\n",i);
      if (runnable_on[i] & (1 << 1))
        curr_pptr += sprintf(curr_pptr, "runnable_on(%d,gpu).\n",i);
      if (runnable_on[i] & (1 << 2))
        curr_pptr += sprintf(curr_pptr, "runnable_on(%d,fpga).\n",i);
    }

    // cu/1
    curr_pptr += sprintf(curr_pptr, "cu(0..%d).\n", total_cus-1);
   
    // cu_type/2
    for(int i=0; i<total_cus; ++i)
      switch(cus[i]) {
        case CPU:
          curr_pptr += sprintf(curr_pptr, "cu_type(%d,cpu).\n",i);
          break;
        case GPU:
          curr_pptr += sprintf(curr_pptr, "cu_type(%d,gpu).\n",i);
          break;
        case FPGA:  
          curr_pptr += sprintf(curr_pptr, "cu_type(%d,fpga).\n",i);
          break;
      }

    // cu_capacity/2 
    for(int i=0; i<total_cus; ++i)     
      curr_pptr += sprintf(curr_pptr,
        "cu_capacity(%d,%d).\n",i,cu_capacity[i]);

    // msg_exch_cost/3
    for(int i=0; i<total_cus; ++i)
      for(int j=0; j<total_cus; ++j)
        curr_pptr += sprintf(curr_pptr,
          "msg_exch_cost(%d,%d,%d).\n",i,j,msg_exch_cost[i][j]);

}

// Restituisce un vettore in cui nella posizione i-esima è conservato il dispositivo su cui deve girare l'attore i-esimo
enum cu_type *ddm_optimize(
    int total_actors,
    struct actor_matrix actors[total_actors][total_actors],
    int tasks_forecast[total_actors])
{
    // tasks_forecast/2
    for(int i=0; i<total_actors; ++i) {
      curr_pptr += sprintf(curr_pptr,
        "tasks_forecast(%d,%d).\n",i,tasks_forecast[i]);
    }    

    for(int i=0; i<total_actors; ++i) {
      for(int j=0; j<total_actors; ++j) {
        // msg_exchange_rate/3
        if(actors[i][j].msg_exchange_rate)
          curr_pptr += sprintf(curr_pptr, 
            "msg_exchange_rate(%d,%d,%d).\n",i,j,actors[i][j].msg_exchange_rate);
      }
    } 

    for(int i=0; i<total_actors; ++i) {
      for(int j=0; j<total_actors; ++j) {
        // mutual_annoyance/3
        if(actors[i][j].annoyance)
          curr_pptr += sprintf(curr_pptr,
            "mutual_annoyance(%d,%d,%d).\n",i,j,actors[i][j].annoyance);
      }
    }

    *curr_pptr = '\0';

    //printf("%s\n", prog_buff);
    //printf("prog_size_approx: %lu, curr_pptr - prog_buff: %lu\n", prog_size_approx, curr_pptr - prog_buff);

    // TODO: add call to get_model (libasp_solver)
    // that returns a clingo model (not an array of strings)
    //char *atom, *snd, *end;
    //for(int i=0; i<total_actors; ++i) {
    //  atom +=7; // skip run_on(
    //  printf("%ld %ld\n", strtol(atom, &snd, 10), strtol(++snd, &end, 10));   
    //}

    return NULL;
}
