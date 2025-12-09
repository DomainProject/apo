#include "asp_solver.h"
#include <clingo.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

struct asp_solver {
	// reference to the global read-only program
	const char *base_source;

	// clingo internal objects
	clingo_control_t *ctl;
	clingo_backend_t *backend;

	// async control
	clingo_solve_handle_t *solve_handle;
	pthread_mutex_t model_lock;
	bool backend_open;

	// solution caching
	clingo_symbol_t *cached_model;
	size_t cached_model_size;
	bool has_solution;

	// timing
	struct timespec start_time;
	double timeout_sec;
};

/*
 * internal helper to panic on clingo errors.
 */
static void check_clingo_panic(void)
{
	clingo_error_t code = clingo_error_code();
	if(code != clingo_error_success) {
		fprintf(stderr, "clingo fatal error [%d]: %s\n", code, clingo_error_message());
		exit(EXIT_FAILURE);
	}
}

/*
 * callback invoked by clingo during the search process.
 * this handles model events to capture the solution.
 */
static bool on_solve_event(clingo_solve_event_type_t type, void *event_data, void *data, bool *go_on)
{
	// we are only interested in model events
	if(type != clingo_solve_event_type_model)
		return true;

	clingo_model_t *model = event_data;
	asp_solver_t *ctx = data;
	size_t count = 0;

	// get the number of symbols in the model
	if(!clingo_model_symbols_size(model, clingo_show_type_shown, &count))
		return false;

	// protect shared memory update
	pthread_mutex_lock(&ctx->model_lock);

	// resize buffer if necessary
	if(ctx->cached_model)
		free(ctx->cached_model);

	if(count > 0) {
		ctx->cached_model = malloc(count * sizeof(clingo_symbol_t));
		if(!ctx->cached_model) {
			// allocation failure in worker thread prevents from continuing
			pthread_mutex_unlock(&ctx->model_lock);
			return false;
		}

		/* retrieve the symbols into our buffer */
		if(!clingo_model_symbols(model, clingo_show_type_shown, ctx->cached_model, count)) {
			free(ctx->cached_model);
			ctx->cached_model = NULL;
			pthread_mutex_unlock(&ctx->model_lock);
			return false;
		}
	} else {
		ctx->cached_model = NULL;
	}

	ctx->cached_model_size = count;
	ctx->has_solution = true;

	pthread_mutex_unlock(&ctx->model_lock);

	/* continue search for better models (optimization) */
	*go_on = true;
	return true;
}

asp_solver_t *asp_solver_create(const char *static_program)
{
	asp_solver_t *ctx;

	ctx = malloc(sizeof(*ctx));
	if(!ctx)
		return NULL;

	memset(ctx, 0, sizeof(*ctx));
	ctx->base_source = static_program;

	if(pthread_mutex_init(&ctx->model_lock, NULL) != 0) {
		free(ctx);
		return NULL;
	}

	return ctx;
}

void asp_solver_destroy(asp_solver_t *ctx)
{
	if(!ctx)
		return;

	if(ctx->solve_handle)
		clingo_solve_handle_close(ctx->solve_handle);

	if(ctx->ctl)
		clingo_control_free(ctx->ctl);

	if(ctx->cached_model)
		free(ctx->cached_model);

	pthread_mutex_destroy(&ctx->model_lock);
	free(ctx);
}

void asp_solver_begin_session(asp_solver_t *ctx)
{
	// reset async state
	if(ctx->solve_handle) {
		clingo_solve_handle_close(ctx->solve_handle);
		ctx->solve_handle = NULL;
	}

	// reset model cache
	pthread_mutex_lock(&ctx->model_lock);
	if(ctx->cached_model) {
		free(ctx->cached_model);
		ctx->cached_model = NULL;
	}
	ctx->cached_model_size = 0;
	ctx->has_solution = false;
	pthread_mutex_unlock(&ctx->model_lock);

	// destroy old control to clear symbol table
	if(ctx->ctl) {
		clingo_control_free(ctx->ctl);
		ctx->ctl = NULL;
	}

	/* create fresh environment.
	 * args: arguments, arg_size, logger, logger_data, message_limit, control
	 */
	clingo_control_new(NULL, 0, NULL, NULL, 20, &ctx->ctl);
	check_clingo_panic();

	clingo_control_add(ctx->ctl, "base", NULL, 0, ctx->base_source);
	check_clingo_panic();

	clingo_control_backend(ctx->ctl, &ctx->backend);
	check_clingo_panic();

	clingo_backend_begin(ctx->backend);
	check_clingo_panic();

	ctx->backend_open = true;
}

static void inject_symbol(clingo_backend_t *bk, const char *pred, const clingo_symbol_t *args, size_t arity)
{
	clingo_symbol_t sym_fun;
	clingo_atom_t atom;
	bool ret;

	ret = clingo_symbol_create_function(pred, args, arity, true, &sym_fun);
	if(!ret)
		check_clingo_panic();

	ret = clingo_backend_add_atom(bk, &sym_fun, &atom);
	if(!ret)
		check_clingo_panic();

	ret = clingo_backend_rule(bk, false, &atom, 1, NULL, 0);
	if(!ret)
		check_clingo_panic();
}
static void check_backend_state(asp_solver_t *ctx)
{
	if(!ctx->backend_open) {
		fprintf(stderr, "asp_solver: fatal error - attempted to inject facts after run/dump.\n");
		fprintf(stderr,
		    "the session is sealed. please call asp_solver_begin_session() to start a new batch.\n");
		exit(EXIT_FAILURE);
	}
}

void asp_inject_fact_u1(asp_solver_t *ctx, const char *predicate, unsigned int v1)
{
	clingo_symbol_t args[1];

	check_backend_state(ctx);

	clingo_symbol_create_number((int)v1, &args[0]);
	inject_symbol(ctx->backend, predicate, args, 1);
}

void asp_inject_fact_u2(asp_solver_t *ctx, const char *predicate, unsigned int v1, unsigned int v2)
{
	clingo_symbol_t args[2];

	check_backend_state(ctx);

	clingo_symbol_create_number((int)v1, &args[0]);
	clingo_symbol_create_number((int)v2, &args[1]);
	inject_symbol(ctx->backend, predicate, args, 2);
}

void asp_inject_fact_u3(asp_solver_t *ctx, const char *predicate, unsigned int v1, unsigned int v2, unsigned int v3)
{
	clingo_symbol_t args[3];

	check_backend_state(ctx);

	clingo_symbol_create_number((int)v1, &args[0]);
	clingo_symbol_create_number((int)v2, &args[1]);
	clingo_symbol_create_number((int)v3, &args[2]);
	inject_symbol(ctx->backend, predicate, args, 3);
}


/*
 * common internal launcher for solve requests
 */
static void internal_start_solve(asp_solver_t *ctx)
{
	clingo_part_t parts[] = {{"base", NULL, 0}};

	if(ctx->backend_open) {
		if(!clingo_backend_end(ctx->backend))
			check_clingo_panic();
		ctx->backend_open = false;
	}

	if(!clingo_control_ground(ctx->ctl, parts, 1, NULL, NULL))
		check_clingo_panic();

	/* async is ALWAYS enabled. this forces clingo to allocate a solve handle,
	   preventing "handle is NULL" errors even for synchronous runs. */
	clingo_solve_mode_bitset_t mode = clingo_solve_mode_async;

	// args: control, mode, assumptions, assump_size, notify, data, handle
	if(!clingo_control_solve(ctx->ctl, mode, NULL, 0, on_solve_event, ctx, &ctx->solve_handle))
		check_clingo_panic();

	if(!ctx->solve_handle) {
		fprintf(stderr, "clingo fatal error: solve handle is NULL after successful solve call\n");
		exit(EXIT_FAILURE);
	}
}

bool asp_solver_run_sync(asp_solver_t *ctx)
{
	clingo_solve_result_bitset_t res;

	// internally async, but we block immediately
	internal_start_solve(ctx);

	// calling get() blocks until finished
	clingo_solve_handle_get(ctx->solve_handle, &res);

	// clean up handle immediately for sync calls
	clingo_solve_handle_close(ctx->solve_handle);
	ctx->solve_handle = NULL;

	return ctx->has_solution;
}

void asp_solver_run_async(asp_solver_t *ctx, double max_wait_seconds)
{
	ctx->timeout_sec = max_wait_seconds;
	clock_gettime(CLOCK_MONOTONIC, &ctx->start_time);

	internal_start_solve(ctx);
}

asp_result_t asp_solver_poll(asp_solver_t *ctx, const clingo_symbol_t **model, size_t *count)
{
	bool done = false;
	struct timespec now;
	double elapsed;
	asp_result_t result = ASP_SEARCHING;

	if(!ctx->solve_handle)
		return ASP_UNSATISFIABLE; // should not happen if used correctly

	// non-blocking check for completion
	clingo_solve_handle_wait(ctx->solve_handle, 0, &done);

	if(done) {
		clingo_solve_result_bitset_t res;
		clingo_solve_handle_get(ctx->solve_handle, &res);

		if(res & clingo_solve_result_satisfiable) {
			result = ASP_OPT; /* finished + sat = optimal/final */
		} else {
			return ASP_UNSATISFIABLE;
		}
	} else {
		// check timeout
		clock_gettime(CLOCK_MONOTONIC, &now);
		elapsed = (double)now.tv_sec - (double)ctx->start_time.tv_sec +
		          (double)(now.tv_nsec - ctx->start_time.tv_nsec) / 1e9;

		if(elapsed > ctx->timeout_sec) {
			clingo_solve_handle_cancel(ctx->solve_handle);
			// we must wait for the cancellation to propagate so the model state is stable.
			clingo_solve_handle_wait(ctx->solve_handle, 0, NULL);

			result = ctx->has_solution ? ASP_TIMEOUT_FOUND : ASP_TIMEOUT_NOSOL;
		} else if(ctx->has_solution) {
			result = ASP_FOUND; // running, but found something
		} else {
			return ASP_SEARCHING;
		}
	}

	// update output pointers if we have a result and a model exists
	if(result != ASP_UNSATISFIABLE && result != ASP_TIMEOUT_NOSOL && result != ASP_SEARCHING) {
		pthread_mutex_lock(&ctx->model_lock);
		if(model)
			*model = ctx->cached_model;
		if(count)
			*count = ctx->cached_model_size;
		pthread_mutex_unlock(&ctx->model_lock);
	}

	return result;
}

bool asp_solver_dump(asp_solver_t *ctx, const char *filepath)
{
	FILE *f;
	clingo_symbolic_atoms_t *atoms;
	clingo_symbolic_atom_iterator_t it, end;
	clingo_part_t parts[] = {{"base", NULL, 0}};

	f = fopen(filepath, "w");
	if(!f)
		return false;

	/* write base program */
	if(ctx->base_source) {
		fprintf(f, "%% Base Program\n%s\n", ctx->base_source);
	}

	/* ensure backend closed and program grounded to inspect atoms */
	if(ctx->backend_open) {
		if(!clingo_backend_end(ctx->backend)) {
			fclose(f);
			return false;
		}
		ctx->backend_open = false;
	}

	/* we must ground to populate the symbolic atoms (signatures and facts) */
	if(!clingo_control_ground(ctx->ctl, parts, 1, NULL, NULL)) {
		fclose(f);
		return false;
	}

	/* iterate facts */
	fprintf(f, "\n%% Injected Facts (Grounder State)\n");
	if(clingo_control_symbolic_atoms(ctx->ctl, &atoms)) {
		if(clingo_symbolic_atoms_begin(atoms, NULL, &it) && clingo_symbolic_atoms_end(atoms, &end)) {
			while(1) {
				bool equal, is_fact;
				if(!clingo_symbolic_atoms_iterator_is_equal_to(atoms, it, end, &equal))
					break;
				if(equal)
					break;

				/* check if the atom is a static fact (true at step 0) */
				if(clingo_symbolic_atoms_is_fact(atoms, it, &is_fact) && is_fact) {
					clingo_symbol_t sym;
					size_t len;
					if(clingo_symbolic_atoms_symbol(atoms, it, &sym)) {
						if(clingo_symbol_to_string_size(sym, &len)) {
							char *buf = malloc(len);
							if(buf) {
								if(clingo_symbol_to_string(sym, buf, len)) {
									fprintf(f, "%s.\n", buf);
								}
								free(buf);
							}
						}
					}
				}

				/* advance iterator */
				if(!clingo_symbolic_atoms_next(atoms, it, &it))
					break;
			}
		}
	}

	fclose(f);
	return true;
}
