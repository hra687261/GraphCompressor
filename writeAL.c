#include "compAdjListG.h"
#include "compAdjListCL.h"
#include "compAdjListCB.h"
#include "compAdjListI.h"

int main(int argc, char **argv)
{
  if (argc < 5 || argc > 9) {
    fprintf(stderr,
      "Missing args! \nexpected: <datastructure_id{0..3}> <edgelist_filepath> \
<output_filepath> <function_id{0..4}> [<k_shrinking_parameter> \
<window_size> <max_ref_count> <threshold>]\n");
    return EXIT_FAILURE;
  }

  uint8_t n_arg = 1;
  uint8_t ds_id = atoi(argv[n_arg++]);

  char *path = argv[n_arg++],
    *output = argv[n_arg++];

  uint64_t u = 0;
  uint8_t func_id = atoi(argv[n_arg++]),
    k = 1;

  if (func_id == 4) {
    k = atoi(argv[n_arg++]);
    setK(k);
  }

  uint64_t ws, mrc, threshold;

  time_t t1, t2, t3;
  if (ds_id == 0) {
    //CompAdjListG

    t1 = time(NULL);
    CompAdjListG *al = load_CompAdjListG(path,
      get_encoding_function(func_id),
      get_expl_function(func_id), &u);

    fprintf(stderr,
      "Loaded CompAdjListG: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
      al->n, al->e, (al->n + 1) * 8, al->nbb, (al->n + 1) * 8 + al->nbb);

    t2 = time(NULL);
    fprintf(stderr,
      "Loading execution time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    write_CompAdjListG(al, func_id, output);

    t3 = time(NULL);
    fprintf(stderr,
      "Writing execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    free_CompAdjListG(al);
    t2 = time(NULL);
    fprintf(stderr,
      "Overall time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  } else if (ds_id == 1) {
    //CompAdjListCL
    
    if (argc < 7 || argc > 8) {
      fprintf(stderr,
        "Missing args! \nexpected: <datastructure_id{0..3}> <edgelist_filepath> \
<output_filepath> <function_id{0..4}> [<k_shrinking_parameter>] <window_size> <max_ref_count>\n");
      return EXIT_FAILURE;
    }

    ws = atoi(argv[n_arg++]);
    mrc = atoi(argv[n_arg++]);

    t1 = time(NULL);
    CompAdjListCL *al = load_CompAdjListCL(path,
      get_encoding_function(func_id),
      get_expl_function(func_id), &u,
      ws, mrc);

    fprintf(stderr,
      "Loaded CompAdjListCL: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
      al->n, al->e, (al->n + 1) * 8, al->nbb, (al->n + 1) * 8 + al->nbb);

    t2 = time(NULL);
    fprintf(stderr,
      "Reading execution time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

    t2 = time(NULL);
    write_CompAdjListCL(al, func_id, output);

    t3 = time(NULL);
    fprintf(stderr,
      "Writing execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    free_CompAdjListCL(al);
    t2 = time(NULL);
    fprintf(stderr,
      "Overall time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  } else if (ds_id == 2) {
    //CompAdjListCB

    if (argc < 7 || argc > 8) {
      fprintf(stderr,
        "Missing args! \nexpected: <datastructure_id{0..3}> <edgelist_filepath> \
<output_filepath> <function_id{0..4}> [<k_shrinking_parameter>] <window_size> <max_ref_count>\n");
      return EXIT_FAILURE;
    }

    ws = atoi(argv[n_arg++]);
    mrc = atoi(argv[n_arg++]);

    t1 = time(NULL);
    CompAdjListCB *al = load_CompAdjListCB(path,
      get_encoding_function(func_id),
      get_expl_function(func_id), &u,
      ws, mrc);

    fprintf(stderr,
      "Loaded CompAdjListCB: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
      al->n, al->e, (al->n + 1) * 8, al->nbb, (al->n + 1) * 8 + al->nbb);

    t2 = time(NULL);
    fprintf(stderr,
      "Reading execution time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

    t2 = time(NULL);
    write_CompAdjListCB(al, func_id, output);

    t3 = time(NULL);
    fprintf(stderr,
      "Writing execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    free_CompAdjListCB(al);
    t2 = time(NULL);
    fprintf(stderr,
      "Overall time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
  } else if (ds_id == 3) {
    //CompAdjListI 


    if (argc < 7 || argc > 8) {
      fprintf(stderr,
        "Missing args! \nexpected: <datastructure_id{0..3}> <edgelist_filepath> \
<output_filepath> <function_id{0..4}> [<k_shrinking_parameter>] <window_size> <max_ref_count>\n");
      return EXIT_FAILURE;
    }

    ws = atoi(argv[n_arg++]);
    mrc = atoi(argv[n_arg++]);
    threshold = atoi(argv[n_arg++]);

    t1 = time(NULL);
    CompAdjListI *al = load_CompAdjListI(path,
      get_encoding_function(func_id),
      get_expl_function(func_id), &u,
      ws, mrc, threshold);

    fprintf(stderr,
      "Loaded CompAdjListI: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
      al->n, al->e, (al->n + 1) * 8, al->nbb, (al->n + 1) * 8 + al->nbb);

    t2 = time(NULL);
    fprintf(stderr,
      "Reading execution time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

    t2 = time(NULL);
    write_CompAdjListI(al, func_id, output);
    t3 = time(NULL);

    fprintf(stderr,
      "Writing execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    free_CompAdjListI(al);
    t2 = time(NULL);
    fprintf(stderr,
      "Overall time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  }
  return EXIT_SUCCESS;
}