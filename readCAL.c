#include "compAdjList.h"

int main(int argc, char **argv) {

  if (argc != 4) {
    fprintf(stderr,
      "Missing args! \nexpected: <CAL_filepath>\
 <bfs-starting-node> <bool: print bfs return>\n");
    return EXIT_FAILURE;
  }

  char *path = argv[1];
  uint8_t p = atoi(argv[3]), id;
  uint64_t firstn = atoi(argv[2]), nbv, *bfs_res;

  time_t t1, t2, t3;
  fprintf(stderr, ">>> cmpfilepath : %s\n\
>>> bfs_s_n : %lu \n>>> p_bfs : %u\n\n", path, firstn, p);

  t1 = time(NULL);
  CompAdjList *cal = read_CompAdjList(path, & id);
  t2 = time(NULL);
  fprintf(stderr,
    "Read CompAdjList: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
    cal->n, cal->e, cal->cds * 8, cal->nbb, cal->cds * 8 + cal->nbb);

  fprintf(stderr,
    "Reading execution time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  t2 = time(NULL);
  bfs_res = bfs_CompAdjList(cal, firstn, & nbv, get_decoding_function(id));
  t3 = time(NULL);

  fprintf(stderr,
    "BFS execution time = %ldh%ldm%lds\n",
    (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

  if (p) {
    fprintf(stderr, "BFS res: ");
    for (uint64_t i = 0; i < nbv; i++)
      fprintf(stderr, "%lu ", bfs_res[i]);
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "BFS length: %lu\n", nbv);

  t2 = time(NULL);
  fprintf(stderr,
    "Overall time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
  free(bfs_res);
  free_CompAdjList(cal);

}