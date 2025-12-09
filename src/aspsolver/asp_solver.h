#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <clingo.h>

/*
 * result codes for the polling interface.
 */
typedef enum {
	ASP_UNSATISFIABLE,
	ASP_FOUND,         /* solution found, but search continues (e.g. optimizing) */
	ASP_OPT,           /* optimal solution found or search exhausted */
	ASP_TIMEOUT_NOSOL, /* timeout reached, no solution found so far */
	ASP_TIMEOUT_FOUND, /* timeout reached, but we have a valid solution */
	ASP_SEARCHING      /* still running, no solution found yet */
} asp_result_t;

/*
 * opaque handle to the solver context.
 */
typedef struct asp_solver asp_solver_t;

/*
 * creates a new solver context.
 * static_program: pointer to the global, null-terminated asp source code.
 */
asp_solver_t *asp_solver_create(const char *static_program);

/*
 * destroys the solver context.
 */
void asp_solver_destroy(asp_solver_t *ctx);

/*
 * begins a new solving session.
 * resets the internal state and loads the base program.
 */
void asp_solver_begin_session(asp_solver_t *ctx);

/*
 * injection functions (arity 1-3).
 */
void asp_inject_fact_u1(asp_solver_t *ctx, const char *pred, unsigned int v1);
void asp_inject_fact_u2(asp_solver_t *ctx, const char *pred, unsigned int v1, unsigned int v2);
void asp_inject_fact_u3(asp_solver_t *ctx, const char *pred, unsigned int v1, unsigned int v2, unsigned int v3);

/*
 * starts the solver in synchronous mode.
 * blocks until the search is complete.
 * returns true if satisfiable, false otherwise.
 */
bool asp_solver_run_sync(asp_solver_t *ctx);

/*
 * starts the solver in asynchronous mode.
 * returns immediately. the max_wait_seconds parameter defines the
 * soft deadline for the search.
 */
void asp_solver_run_async(asp_solver_t *ctx, double max_wait_seconds);

/*
 * checks the status of the asynchronous solver.
 *
 * model: output pointer to the array of symbols (the solution).
 * count: output pointer to the number of symbols in the model.
 *
 * returns the current status of the search. if ASP_FOUND, ASP_OPT,
 * or ASP_TIMEOUT_FOUND is returned, the 'model' and 'count' pointers
 * are updated with the latest solution found.
 */
asp_result_t asp_solver_poll(asp_solver_t *ctx, const clingo_symbol_t **model, size_t *count);

/*
 * dumps the current program (base source + injected facts) to a file.
 * useful for debugging.
 *
 * note: this function triggers grounding of the program to inspect the
 * injected facts.
 *
 * returns true on success, false on failure (file io or clingo error).
 */
bool asp_solver_dump(asp_solver_t *ctx, const char *filepath);
