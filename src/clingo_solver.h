// #include <clingo.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <string.h>
#include <clingo.h>

typedef struct cctx {
	clingo_control_t *ctl;
	clingo_solve_handle_t *handle;
	int parts_l;
	clingo_part_t *parts;
} clingo_ctx;

extern bool init_clingo(char const *program, int argc, char const **argv, clingo_ctx **cs);
extern bool init_clingo_progfilename(char const *progfilename, int argc, char const **argv, clingo_ctx **cctx);
extern bool get_model(clingo_ctx *cs, clingo_model_t const **model);
extern bool clingo_model_to_strings(clingo_model_t const *model, size_t *as_n, char **as[]);
extern void print_as(size_t as_n, char **as);
extern void print_model(clingo_model_t const *model);
extern bool free_clingo(clingo_ctx *cs);
