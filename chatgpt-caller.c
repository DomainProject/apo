#include <stdio.h>
#include <stdlib.h>
#include <clingo.h>

void run_clingo_optimization(const char *asp_file, const char *params) {
    clingo_control_t *ctl = NULL;
    clingo_model_t const **models;
    clingo_configuration_t *conf;
    clingo_control_new(NULL, 0, NULL, NULL, 20, &ctl);

    // Load the ASP program from the file
    clingo_control_load(ctl, asp_file);

    // Add any parameters to the ASP program
    clingo_control_add(ctl, "base", NULL, 0, params);

    // Ground the program
    clingo_part_t parts[] = {{ "base", NULL, 0 }};
    clingo_control_ground(ctl, parts, 1, NULL, NULL);

    // Obtain the configuration
    clingo_control_configuration(ctl, &conf);

    // Set the solving mode to optimization
    clingo_configuration_root(conf, NULL);
    clingo_configuration_value_set(conf, "opt_mode", "opt", NULL);

    // Solve the grounded program
    clingo_solve_handle_t *handle;
    clingo_control_solve(ctl, clingo_solve_mode_yield, NULL, 0, NULL, &handle);

    // Process the solution
    clingo_solve_result_bitset_t result;
    clingo_solve_handle_get(handle, &result);
    if (result & clingo_solve_result_satisfiable) {
        clingo_model_t const *model;
        while (clingo_solve_handle_model(handle, &model) == clingo_solve_iterate) {
            // Print the optimized model
            char *model_str;
            size_t model_str_len;
            clingo_model_to_string_size(model, &model_str_len);
            model_str = malloc(model_str_len);
            clingo_model_to_string(model, model_str, model_str_len);
            printf("Optimized Model: %s\n", model_str);
            free(model_str);
        }
    }

    // Clean up
    clingo_solve_handle_close(handle);
    clingo_control_free(ctl);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <asp_file> <params>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *asp_file = argv[1];
    const char *params = argv[2];

    run_clingo_optimization(asp_file, params);

    return EXIT_SUCCESS;
}
