#include "clingo_solver.h"


bool clingo_model_to_strings(clingo_model_t const *model, size_t *as_n, char **as[]) {
  bool ret = true;
  clingo_symbol_t *atoms = NULL;
  size_t atoms_n;
  clingo_symbol_t const *it, *ie;
  char *str = NULL;
  char **as_tmp = NULL;

  // determine the number of (shown) symbols in the model
  if (!clingo_model_symbols_size(model, clingo_show_type_shown, &atoms_n)) { goto error; }

  // allocate required memory to hold all the symbols
  if (!(atoms = (clingo_symbol_t*)malloc(sizeof(*atoms) * atoms_n))) {
    clingo_set_error(clingo_error_bad_alloc, "could not allocate memory for atoms");
    goto error;
  }

  // allocate required memory to hold all the symbols strings
  if (!(as_tmp = (char**)malloc(sizeof(char*) * atoms_n))) {
    printf("could not allocate memory to store strings representing atoms in as");
    goto error;
  } 

  // retrieve the symbols in the model
  if (!clingo_model_symbols(model, clingo_show_type_shown, atoms, atoms_n)) { goto error; }   

  int i=0;
  for (it = atoms, ie = atoms + atoms_n; it != ie; ++it, ++i) {
    size_t n;

    // determine size of the string representation of the next symbol in the model
    if (!clingo_symbol_to_string_size(*it, &n)) { goto error; }

    // allocate required memory to hold the symbol's string
    if (!(as_tmp[i] = (char*)malloc(sizeof(char) * n))) {
      printf("could not allocate memory to store string representing atom");
      goto error;
    }

    // retrieve the symbol's string
    if (!clingo_symbol_to_string(*it, as_tmp[i], n)) { goto error; }

  }

  goto out;

error:
  ret = false;

out:
  // number of atoms in the model
  *as_n = atoms_n;
  *as = as_tmp;
  if (atoms) { free(atoms); }
  if (str)   { free(str); }

  return ret;
}


bool init_clingo_progfilename(char const *progfilename, int argc, char const **argv, clingo_ctx **cctx) {
  
  int fd = open(progfilename, O_RDONLY);
  if ( fd == -1) {
    printf("%s does not exist!\n", progfilename);
    return false;
  }

  int len = lseek(fd, 0, SEEK_END);
  void *data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
 
  clingo_ctx *cctx_tmp;
  return init_clingo(((char *)data), argc, argv, cctx);
}


bool init_clingo(char const *program, int argc, char const **argv, clingo_ctx **cctx) {
  char const *error_message;
  bool ret = true;
  
  clingo_ctx *cctx_tmp;
  if (!(cctx_tmp = (clingo_ctx *)malloc(sizeof(clingo_ctx)))) {
    clingo_set_error(clingo_error_bad_alloc, "could not allocate memory for atoms");
    goto error;
  } 
  cctx_tmp -> ctl = NULL;
  cctx_tmp -> handle = NULL ;
  cctx_tmp -> parts_l = 1;
  cctx_tmp -> parts = (clingo_part_t *)malloc(sizeof(clingo_part_t) * cctx_tmp -> parts_l); 
  // parts[0]
  char * tmp_name = (char *)malloc(sizeof(char *) * 5);
  strcpy(tmp_name, "base"); 
  cctx_tmp -> parts[0].name = tmp_name;
  cctx_tmp -> parts[0].params = NULL;
  cctx_tmp -> parts[0].size = 0;

  // create a control object and pass command line arguments
  if (!clingo_control_new(argv, argc, NULL, NULL, 20, &cctx_tmp->ctl) != 0) { goto error; }

  // add a logic program to the base part
  if (!clingo_control_add(cctx_tmp->ctl, "base", NULL, 0, program)) { goto error; }

  // ground the base part
  if (!clingo_control_ground(cctx_tmp->ctl, cctx_tmp->parts, 1, NULL, NULL)) { goto error; }

  // get a solve handle
  if (!clingo_control_solve(cctx_tmp->ctl, clingo_solve_mode_yield, NULL, 0, NULL, NULL, &cctx_tmp->handle)) { goto error; }

  goto out;

error:
  if (!(error_message = clingo_error_message())) { error_message = "error"; }

  printf("%s\n", error_message);
  ret = clingo_error_code();

out:
  *cctx = cctx_tmp;

  return ret;
}

bool get_model(clingo_ctx *cctx, clingo_model_t const **model) {

  char const *error_message;

  // get a model
  if (!clingo_solve_handle_resume(cctx->handle)) { goto error; }
  if (!clingo_solve_handle_model(cctx->handle, model)) { goto error; }

  if ( *model )
    return true;
  else
    return false;

error:
  if (!(error_message = clingo_error_message())) { error_message = "error"; }

  printf("%s\n", error_message);
  return clingo_error_code();

}



void print_as(size_t as_n, char **as) {
  printf("Model:");
  for(int i=0; i<as_n; ++i)
    printf(" %s", as[i]);
  printf("\n");
}


void print_model(clingo_model_t const *model) {
  size_t as_n;
  char **as;
  clingo_model_to_strings(model, &as_n, &as);
  print_as(as_n,as);
}


bool free_clingo(clingo_ctx *cctx) {

  clingo_solve_result_bitset_t solve_ret;
  char const *error_message;
 
  // close the solve handle
  if (!clingo_solve_handle_get(cctx->handle, &solve_ret)) { goto error; }   

  // free the solve handle
  if (!clingo_solve_handle_close(cctx->handle)) { goto error; }

  // free the control structure created with clingo_control_new 
  if (cctx->ctl) { clingo_control_free(cctx->ctl); }

  // free clingo_ctx created with clingo_init
  for(int i=0; i<cctx->parts_l; ++i)
    free((char *)cctx->parts[i].name);  

  // free the array of clingo_part created with clingo_init
  free(cctx->parts);
  free(cctx);

  return true;

error:
  if (!(error_message = clingo_error_message())) { error_message = "error"; }

  printf("%s\n", error_message);
  return clingo_error_code();
}