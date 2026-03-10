#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ddm.h"
#include "dynstr.h"
#include "lp/assets.h"
#include "aspsolver/asp_solver.h"

#include <string.h>
#include <unistd.h>

#define TIMEOUT 2.0
#define DUMP_PROGRAM

static const unsigned char *base_program = LDVAR(ddm_asp);
asp_solver_t *solver;
struct dynstr *clingo_base_program_buffer;


static void get_pairs(const clingo_symbol_t *atoms, size_t atoms_n, int **pairs)
{
	clingo_symbol_t const *it, *ie;
	char *str = NULL;

	*pairs = malloc(sizeof(int) * atoms_n);
	if(!*pairs) {
		perror("ddm_init: could not allocate memory for prog_buff");
		return;
	}

	for(it = atoms, ie = atoms + atoms_n; it != ie; ++it) {
		size_t n;

		// determine size of the string representation of the next symbol in the model
		if(!clingo_symbol_to_string_size(*it, &n)) {
			goto error;
		}

		str = malloc(n);
		if(!str) {
			goto error;
		}

		// retrieve the symbol's string
		if(!clingo_symbol_to_string(*it, str, n)) {
			goto error;
		}

		if(strncmp(str, "run_on(", 7) == 0) {
			char *atom = str + 7;
			char *snd, *end;
			int idx = (int)strtol(atom, &snd, 10);
			(*pairs)[idx] = (int)strtol(++snd, &end, 10);
		}
		free(str);
		str = NULL;
	}
	return;

error:
	if(str)
		free(str);
	free(*pairs);
	*pairs = NULL;
}
void ddm_init(int total_cus, int total_actors, const enum cu_type *cus, int msg_exch_cost[total_cus][total_cus],
    short runnable_on[total_actors])
{
	size_t len = LDLEN(ddm_asp);
	dynstr_init(&clingo_base_program_buffer, len);
	dynstr_strcat(clingo_base_program_buffer, (const char *)base_program, len);
	dynstr_printcat(clingo_base_program_buffer, "cu(0..%d).\n", total_cus - 1);
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
	for(int i = 0; i < total_cus; ++i)
		for(int j = 0; j < total_cus; ++j)
			dynstr_printcat(clingo_base_program_buffer, "msg_exch_cost(%d,%d,%d).\n", i, j,
			    msg_exch_cost[i][j]);

	dynstr_printcat(clingo_base_program_buffer, "actor(0..%d).\n", total_actors - 1);
	for(int i = 0; i < total_actors; ++i)
		dynstr_printcat(clingo_base_program_buffer, "runnable_on(%d,%d).\n", i, runnable_on[i]);

	solver = asp_solver_create(dynstr_getbuff(clingo_base_program_buffer));
}


void ddm_optimize(int total_actors, struct actor_matrix actors[total_actors][total_actors],
    int tasks_forecast[total_actors], int total_cus, int cu_capacity[total_cus])
{
	// Reset solver
	asp_solver_begin_session(solver);

	// Inject facts
	for (int i = 0; i < total_actors; ++i) {
		asp_inject_fact_u2(solver, "tasks_forecast", i, tasks_forecast[i]);
	}
	for (int i = 0; i < total_cus; ++i) {
		asp_inject_fact_u2(solver, "cu_capacity", i, cu_capacity[i]);
	}
	for (int i = 0; i < total_actors; ++i) {
		for (int j = 0; j < total_actors; ++j) {
			if (actors[i][j].msg_exchange_rate) {
				asp_inject_fact_u3(solver, "msg_exch_rate", i, j, actors[i][j].msg_exchange_rate);
			}
			if (actors[i][j].annoyance) {
				asp_inject_fact_u3(solver, "mutual_annoyance", i, j, actors[i][j].annoyance);
			}
		}
	}

#ifdef DUMP_PROGRAM
	asp_solver_dump(solver, "ddm_tmp.asp");
#endif

	asp_solver_run_async(solver, TIMEOUT);
}

enum result ddm_poll(int **assignment)
{
	const clingo_symbol_t *model;
	size_t count;
	asp_result_t status = asp_solver_poll(solver, &model, &count);
	if (status == ASP_TIMEOUT_FOUND || status == ASP_OPT) {
		get_pairs(model, count, assignment);
		return FOUND;
	}
	if(status == ASP_TIMEOUT_NOSOL) {
		return TIMEOUT;
	} else if(status == ASP_UNSATISFIABLE) {
		return UNSAT;
	} 
	
	return SEARCHING;
	
}

void ddm_free_assignment(int *assignment)
{
	free(assignment);
}

void ddm_destroy(void)
{
	asp_solver_destroy(solver);
}
