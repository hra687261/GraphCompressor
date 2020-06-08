#include "compAdjList.h"

int main(int argc, char ** argv)
{
  if (argc < 8 || argc > 9) {
    fprintf(stderr,
      "Missing args! \nexpected: <edgelist_filepath> <output_filepath> \
<function_id{0..4}> [<k_shrinking_parameter(>0)> <window_size(>0)> \
<max_ref_count(>0)> <threshold(>=2)> <jump-len(>0)>]\n");
    return EXIT_FAILURE;
  }
  uint8_t n_arg = 1;
  char *path = argv[n_arg++],
    *output = argv[n_arg++];

  uint64_t u = 0;
  uint8_t func_id = atoi(argv[n_arg++]), k = 1;

  if (func_id == 4) {
    k = atoi(argv[n_arg++]);
    setK(k);
  }

  uint64_t ws = atoi(argv[n_arg++]),
    mrc = atoi(argv[n_arg++]),
    threshold = atoi(argv[n_arg++]),
    jump = atoi(argv[n_arg++]);

  time_t t1, t2, t3;

  t1 = time(NULL);
  CompAdjList * cal = load_CompAdjList(path, get_encoding_function(func_id),
    get_expl_function(func_id), &u, jump, ws, mrc, threshold);

  fprintf(stderr,
    "CAL: n=%lu  e=%lu  size(cd)=%lu B  size(adj)=%lu B  sizesum=%lu B\n",
    cal->n, cal->e, cal->cds * 8, cal->nbb, cal->cds * 8 + cal->nbb);

  fprintf(stderr,
    "Loaded CompAdjList: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
    cal->n, cal->e, cal->cds, cal->nbb, cal->cds * 8 + cal->nbb);

  t2 = time(NULL);
  fprintf(stderr,
    "Loading execution time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  write_CompAdjList(cal, func_id, output);

  t3 = time(NULL);
  fprintf(stderr,
    "Writing execution time = %ldh%ldm%lds\n",
    (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

  free_CompAdjList(cal);
  t2 = time(NULL);
  fprintf(stderr,
    "Overall time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  return EXIT_SUCCESS;
}