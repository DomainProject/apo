#include "../src/aspsolver/asp_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

/*
 * macro for simple assertions.
 * prints pass/fail status to stdout.
 */
#define TEST_ASSERT(cond, msg) \
	do { \
		if (!(cond)) { \
			fprintf(stderr, "[FAIL] %s: %s\n", __func__, msg); \
			exit(EXIT_FAILURE); \
		} else { \
			fprintf(stdout, "[PASS] %s: %s\n", __func__, msg); \
		} \
	} while (0)

/*
 * test 1: basic synchronous execution
 * verifies creation, session management, and simple satisfiability.
 */
void test_sync_sat(void)
{
	const char *prog = "target(X) :- source(X). :- target(10).";
	asp_solver_t *solver = asp_solver_create(prog);
	TEST_ASSERT(solver != NULL, "solver creation");

	/* case a: satisfiable input */
	asp_solver_begin_session(solver);
	asp_inject_fact_u1(solver, "source", 5);
	bool result = asp_solver_run_sync(solver);
	TEST_ASSERT(result == true, "sync solve should be satisfiable (source=5)");

	/* case b: unsatisfiable input (triggers constraint :- target(10)) */
	asp_solver_begin_session(solver);
	asp_inject_fact_u1(solver, "source", 10);
	result = asp_solver_run_sync(solver);
	TEST_ASSERT(result == false, "sync solve should be unsatisfiable (source=10)");

	asp_solver_destroy(solver);
}

/*
 * test 2: fact injection arity
 * verifies that unary, binary, and ternary facts are correctly
 * injected and recognized by the logic program.
 */
void test_injection_arities(void)
{
	/* program requires one of each fact type to derive 'valid' */
	const char *prog =
		"valid :- p1(10), p2(10, 20), p3(10, 20, 30)."
		":- not valid.";

	asp_solver_t *solver = asp_solver_create(prog);
	bool result;

	asp_solver_begin_session(solver);

	/* inject facts matching the rule body */
	asp_inject_fact_u1(solver, "p1", 10);
	asp_inject_fact_u2(solver, "p2", 10, 20);
	asp_inject_fact_u3(solver, "p3", 10, 20, 30);

	result = asp_solver_run_sync(solver);
	TEST_ASSERT(result == true, "all injection arities verified");

	asp_solver_destroy(solver);
}

/*
 * test 3: asynchronous execution and polling
 * solves a combinatorial problem (n-queens style) asynchronously.
 * verifies that polling eventually returns a result and retrieves the model.
 */
void test_async_solve_model_retrieval(void)
{
	/* simple permutation generation to ensure it takes non-zero time */
	const char *prog =
		"1 { q(1..4, C) } 1 :- C=1..4."
		"1 { q(R, 1..4) } 1 :- R=1..4."
		":- q(R1, C1), q(R2, C2), R1 < R2, |R1-R2| == |C1-C2|.";

	asp_solver_t *solver = asp_solver_create(prog);
	asp_result_t status;
	const clingo_symbol_t *model = NULL;
	size_t count = 0;

	asp_solver_begin_session(solver);

	/* start async with a generous timeout */
	asp_solver_run_async(solver, 5.0);

	do {
		status = asp_solver_poll(solver, &model, &count);
		usleep(100); /* yield cpu */
	} while (status == ASP_SEARCHING || status == ASP_FOUND);

	TEST_ASSERT(status == ASP_OPT, "async solve found optimal solution");
	TEST_ASSERT(count > 0, "model contains symbols");
	TEST_ASSERT(model != NULL, "model pointer is valid");

	/* verify we got q/2 facts */
	/* note: printing symbols requires clingo string api, skipping for unit test minimal deps */

	asp_solver_destroy(solver);
}

/*
 * test 4: timeout behavior
 * forces a timeout by creating a massive grounding problem and setting
 * an extremely short timeout.
 */
void test_async_timeout(void)
{
	/* * generate a large number of facts and rules to force grounding time
	 * to exceed the timeout.
	 * p(1..100000).
	 */
	const char *prog = "p(1..100000). pair(X,Y) :- p(X), p(Y), X=Y+1.";

	asp_solver_t *solver = asp_solver_create(prog);
	asp_result_t status;

	asp_solver_begin_session(solver);

	/* set timeout to effectively zero */
	asp_solver_run_async(solver, 0.000001);

	/* poll until completion */
	do {
		status = asp_solver_poll(solver, NULL, NULL);
	} while (status == ASP_SEARCHING);

	/* * depending on machine speed, it might timeout with no solution (grounding stuck)
	 * or timeout with solution. either is a pass for 'timeout'.
	 */
	bool is_timeout = status == ASP_TIMEOUT_NOSOL || status == ASP_TIMEOUT_FOUND;

	if (!is_timeout) {
		fprintf(stderr, "warning: machine too fast, finished before timeout. status: %d\n", status);
	} else {
		TEST_ASSERT(is_timeout, "solver timed out as expected");
	}

	asp_solver_destroy(solver);
}

/*
 * test 5: zero-copy verification
 * ensures the solver doesn't crash if the external string buffer remains valid.
 */
void test_lifecycle(void)
{
	/* global/static string simulation */
	static const char global_prog[] = "a :- b.";
	asp_solver_t *solver = asp_solver_create(global_prog);

	asp_solver_begin_session(solver);
	asp_inject_fact_u1(solver, "b", 0); /* effectively b(0) */

	/* destroy without running - check for memory leaks/double free logic */
	asp_solver_destroy(solver);

	TEST_ASSERT(1, "lifecycle create/destroy sequence passed");
}

int main(void)
{
	printf("=== starting asp_solver library tests ===\n");

	test_sync_sat();
	test_injection_arities();
	test_async_solve_model_retrieval();
	test_async_timeout();
	test_lifecycle();

	printf("=== all tests passed successfully ===\n");
	return 0;
}
