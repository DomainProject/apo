#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ddm.h"

// gcc ddm_driver.c ddm.c -I/opt/homebrew/Cellar/clingo/5.6.2/include -I/Users/emanuele/repos/ABALearn/miscellanea/libasp_solver  -L/opt/homebrew/Cellar/clingo/5.6.2/lib -L/Users/emanuele/repos/ABALearn/miscellanea/libasp_solver -lclingo_solver -lclingo

static int num_cus;
static enum cu_type *mapping_cus;
static int *cu_capacity; 

char *prog_buff;
char *curr_pptr;
size_t prog_size_approx;

bool get_pairs(clingo_model_t const *model);

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

    FILE *file = fopen("../lp/ddm_v4.asp", "r");
    if (file == NULL) {
       perror("Error opening file");
       exit(errno);
    }

    // Moving pointer to end
    fseek(file, 0, SEEK_END);

    // Getting position of pointer
    int len = ftell(file);
   
    // Compute an overapproximation of the memory required to store the ASP program
    prog_size_approx = total_actors * (100 * total_actors + 120) + 
                       total_cus * (50 * total_cus + 30) + 60 + len;

    if (!(prog_buff = malloc(prog_size_approx))) {
      perror("ddm_init: could not allocate memory for prog_buff");
      exit(errno);
    }

    curr_pptr = prog_buff;
    
    // reading core ddm ASP program  
    // Moving pointer to end
    fseek(file, 0, SEEK_SET);
    curr_pptr += fread(curr_pptr, 1, len, file);

    fclose(file);

    // adding facts
    curr_pptr += sprintf(curr_pptr, "%%%%%% facts\n");
    
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
            "msg_exch_rate(%d,%d,%d).\n",i,j,actors[i][j].msg_exchange_rate);
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

    FILE *file = fopen("ddm_tmp.asp", "w");
    if (file == NULL) {
       perror("Error opening file");
       exit(errno);
    }
    fprintf(file, "%s\n", prog_buff);
    fclose(file);
    printf("prog_size_approx: %lu, curr_pptr - prog_buff: %lu\n", prog_size_approx, curr_pptr - prog_buff);

    
    // 1. initialize clingo w/program in prog_buff
    const char *argv[] = { "--opt-mode", "opt" };
    const int argc = 2;
    clingo_ctx *ctxt;
    init_clingo(prog_buff, argc, argv, &ctxt);
 
    // 2. invoke clingo & get the optimal as
    clingo_model_t const *model, *tmp_model;
    while( get_model(ctxt, &tmp_model) ) 
      model = tmp_model;

    // 3. extract pairs <actor,cu> from the as (run_on/2 facts)
    get_pairs(model); 
    
 
    // alernative code using optN and clingo_model_optimality_proven */
    /*
    // 1. initialize clingo w/program in prog_buff
    const char *argv[] = { "--opt-mode", "optN" };
    const int argc = 2;
    clingo_ctx *ctxt;
    init_clingo(prog_buff, argc, argv, &ctxt);
 
    // 2. invoke clingo & get the optimal as
    clingo_model_t const *model; 
    bool proven;
    while( get_model(ctxt, &model) ) {
      if ( clingo_model_optimality_proven(model,&proven) ) {
        if(proven) {
        // 3. extract pairs <actor,cu> from the as (run_on/2 facts)
          get_pairs(model); 
          break;
        }
      } else {
        perror("clingo_model_optimality_proven error ");
        exit(errno);
      }
    }
    */

    // 4. free clingo
    free_clingo(ctxt);

    return NULL;
}


bool get_pairs(clingo_model_t const *model) {
  bool ret = true;
  clingo_symbol_t *atoms = NULL;
  size_t atoms_n;
  clingo_symbol_t const *it, *ie;
  char str[50];

  // determine the number of (shown) symbols in the model
  if (!clingo_model_symbols_size(model, clingo_show_type_shown, &atoms_n)) { goto error; }

  // allocate required memory to hold all the symbols
  if (!(atoms = (clingo_symbol_t*)malloc(sizeof(*atoms) * atoms_n))) {
    clingo_set_error(clingo_error_bad_alloc, "could not allocate memory for atoms");
    goto error;
  }

  // retrieve the symbols in the model
  if (!clingo_model_symbols(model, clingo_show_type_shown, atoms, atoms_n)) { goto error; }   

  int i=0;
  for (it = atoms, ie = atoms + atoms_n; it != ie; ++it, ++i) {
    size_t n;

    // determine size of the string representation of the next symbol in the model
    if (!clingo_symbol_to_string_size(*it, &n)) { goto error; }

    // retrieve the symbol's string
    if (!clingo_symbol_to_string(*it, str, n)) { goto error; }

    char *atom = str + 7; // skip run_on(
    char *snd, *end;
    printf("%ld %ld\n", strtol(atom, &snd, 10), strtol(++snd, &end, 10));     

  }

  goto out;

error:
  ret = false;

out:
  // number of atoms in the model
  if (atoms) { free(atoms); }

  return ret;
}