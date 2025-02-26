#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "ddm.h"
#include "dynstr.h"
#include "lp/assets.h"

#define TIMEOUT 1
#define USE_ASSETS

#ifndef USE_ASSETS
static unsigned char *base_program = NULL;
#else
static const unsigned char *base_program = LDVAR(ddm_asp);
#endif

static clingo_ctx *cctx;
struct dynstr *clingo_base_program_buffer;
struct dynstr *clingo_program_buffer;

static int *get_pairs(clingo_model_t const *model)
{
	clingo_symbol_t *atoms = NULL;
	size_t atoms_n;
	clingo_symbol_t const *it, *ie;
	char str[50];

	// determine the number of (shown) symbols in the model
	if(!clingo_model_symbols_size(model, clingo_show_type_shown, &atoms_n)) {
		goto error3;
	}

	// allocate required memory to hold all the symbols
	if(!(atoms = malloc(sizeof(*atoms) * atoms_n))) {
		clingo_set_error(clingo_error_bad_alloc, "could not allocate memory for atoms");
		goto error2;
	}

	int *pairs = malloc(sizeof(int) * atoms_n);
	if(!pairs) {
		perror("ddm_init: could not allocate memory for prog_buff");
		goto error;
	}

	// retrieve the symbols in the model
	if(!clingo_model_symbols(model, clingo_show_type_shown, atoms, atoms_n)) {
		goto error;
	}

	for(it = atoms, ie = atoms + atoms_n; it != ie; ++it) {
		size_t n;

		// determine size of the string representation of the next symbol in the model
		if(!clingo_symbol_to_string_size(*it, &n)) {
			goto error;
		}

		// retrieve the symbol's string
		if(!clingo_symbol_to_string(*it, str, n)) {
			goto error;
		}

		char *atom = str + 7; // skip run_on(
		char *snd, *end;
		int idx = (int)strtol(atom, &snd, 10);
		pairs[idx] = (int)strtol(++snd, &end, 10);
	}

	// number of atoms in the model
	free(atoms);
	return pairs;

error:
	free(pairs);
error2:
	free(atoms);
error3:
	return NULL;
}


void ddm_init(int total_cus, int total_actors, const enum cu_type *cus, int msg_exch_cost[total_cus][total_cus],
    short runnable_on[total_actors])
{
	/*  ------------- BEGIN TEMPORARY CODE -----------  */
#ifndef USE_ASSETS
	int fd = open("lp/ddm.asp", O_RDONLY);
	if(fd == -1) {
		perror("open error");
		exit(errno);
	}

	int len = lseek(fd, 0, SEEK_END);
	base_program = (unsigned char *)mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);

	// the code above should be replaced by:
#else
	size_t len = LDLEN(ddm_asp);
#endif
	/*  ------------- END TEMPORARY CODE -------------  */

	// Copy the ASP rules to solve the optimization problem to prog_buff
	dynstr_init(&clingo_base_program_buffer, len);
	dynstr_strcat(clingo_base_program_buffer, base_program, len);

	/* --- adding facts --- */
	dynstr_printcat(clingo_base_program_buffer, "\n%%%%%% facts\n");

	/* --- HW platform dependent facts --- */
	// cu/1
	dynstr_printcat(clingo_base_program_buffer, "cu(0..%d).\n", total_cus - 1);

	// cu_type/2
	for(int i = 0; i < total_cus; ++i)
		switch(cus[i]) {
			case CPU:
				dynstr_printcat(clingo_base_program_buffer, "cu_type(%d,cpu).\n", i);
				break;
			case GPU:
				dynstr_printcat(clingo_base_program_buffer, "cu_type(%d,gpu).\n", i);
				break;
			case FPGA:
				dynstr_printcat(clingo_base_program_buffer, "cu_type(%d,fpga).\n", i);
				break;
		}

	// msg_exch_cost/3
	for(int i = 0; i < total_cus; ++i)
		for(int j = 0; j < total_cus; ++j)
			dynstr_printcat(clingo_base_program_buffer, "msg_exch_cost(%d,%d,%d).\n", i, j,
			    msg_exch_cost[i][j]);

	/* --- Actors dependent facts --- */
	// actor/1
	dynstr_printcat(clingo_base_program_buffer, "actor(0..%d).\n", total_actors - 1);

	// runnable_on_class/2
	for(int i = 0; i < total_actors; ++i)
		dynstr_printcat(clingo_base_program_buffer, "runnable_on(%d,%d).\n", i, runnable_on[i]);
}


// Restituisce un vettore in cui nella posizione i-esima è conservato il dispositivo su cui deve girare l'attore i-esimo
void ddm_optimize(int total_actors, struct actor_matrix actors[total_actors][total_actors],
    int tasks_forecast[total_actors], int total_cus, int cu_capacity[total_cus])
{
	// Reset program buffer
	dynstr_strcpy(&clingo_program_buffer, clingo_base_program_buffer);

	// tasks_forecast/2
	for(int i = 0; i < total_actors; ++i)
		dynstr_printcat(clingo_program_buffer, "tasks_forecast(%d,%d).\n", i, tasks_forecast[i]);

	// cu_capacity/2
	for(int i = 0; i < total_cus; ++i)
		dynstr_printcat(clingo_program_buffer, "cu_capacity(%d,%d).\n", i, cu_capacity[i]);

	for(int i = 0; i < total_actors; ++i) {
		for(int j = 0; j < total_actors; ++j) {
			// msg_exchange_rate/3
			if(actors[i][j].msg_exchange_rate) {
				dynstr_printcat(clingo_program_buffer, "msg_exch_rate(%d,%d,%d).\n", i, j,
				    actors[i][j].msg_exchange_rate);
			}
		}
	}

	for(int i = 0; i < total_actors; ++i) {
		for(int j = 0; j < total_actors; ++j) {
			// mutual_annoyance/3
			if(actors[i][j].annoyance) {
				dynstr_printcat(clingo_program_buffer, "mutual_annoyance(%d,%d,%d).\n", i, j,
				    actors[i][j].annoyance);
			}
		}
	}

#ifndef USE_ASSETS
	FILE *file = fopen("ddm_tmp.asp", "w");
	if(file == NULL) {
		perror("Error opening file");
		exit(errno);
	}
	fprintf(file, "%s\n", dynstr_getbuff(clingo_program_buffer));
	fclose(file);
#endif

	// initialize clingo w/program in prog_buff
	const char *argv[] = {"--opt-mode", "opt"};
	const int argc = 2;
	init_clingo_mode(dynstr_getbuff(clingo_program_buffer), argc, argv, clingo_solve_mode_async | clingo_solve_mode_yield, &cctx);

	// get the first model
	if(!clingo_solve_handle_resume(cctx->handle)) {
		perror(clingo_error_message());
		exit(clingo_error_code());
	}
}

static int *ddm_poll_internal(bool stop_on_optimal)
{
	// 1. invoke clingo & get the optimal as
	static clingo_model_t const *model = NULL;
	clingo_model_t const *tmp_model = NULL;
	bool result;
	// bool proven;
	// size_t costs_size = 3;
	// int64_t *costs = (int64_t *)malloc(sizeof(int64_t) * costs_size);

	// poll clingo to check if a result is ready
	clingo_solve_handle_wait(cctx->handle, 0, &result);

	// check whether the search has finished
	if(result) {
		if(!clingo_solve_handle_model(cctx->handle, &tmp_model)) {
			perror(clingo_error_message());
			exit(clingo_error_code());
		}
		// replace model with the last one (NULL means there are no more models)
		if(tmp_model) {
			model = tmp_model;
			/*
			clingo_model_cost(model, costs, costs_size);
			printf("costs: ");
		for(int i=0; i<costs_size; ++i)
			        printf("%li ", costs[i]);
			printf("\n");
			*/
			// if stop_on_optimal is set, then resume the search for the next model & return NULL
			if(stop_on_optimal) {
				if(!clingo_solve_handle_resume(cctx->handle)) {
					perror(clingo_error_message());
					exit(clingo_error_code());
				}
				return NULL;
			}
		}
		// tmp_model == NULL: there are no more models (the last found is the optimal model) OR
		// tmp_model != NULL && stop_on_optimal == false: a model has been found and it does no matter if it is
		// the optimal model
		// 2. extract pairs <actor,cu> from the as (run_on/2 facts)
		return get_pairs(model);
	}

	// no result (yet)
	return NULL;
}


int *ddm_poll(void)
{
	static time_t last_call = 0;
	time_t now = time(NULL);
	int *ret;

	if(last_call == 0) // First invocation
		last_call = now;

	if((now - last_call) >= TIMEOUT) { // time returns a UNIX timestamp, so this waits at most TIMEOUT seconds
		// If the timeout is too short, we might not have found even a single solution. Stop the world until
		// the first usable solution is found.
		// TODO: this may now be the best suited solution.
		do {
			ret = ddm_poll_internal(false);
		} while(ret == NULL);
		last_call = 0; // Reset the timer for future invocations
	} else {
		ret = ddm_poll_internal(true);
	}

	if(ret != NULL) { // We have found a solution that meets the timing or optimatily requirements: prepare for next
		          // optimization.
		free_clingo(cctx);
		dynstr_fini(&clingo_program_buffer);
	}
	return ret;
}
