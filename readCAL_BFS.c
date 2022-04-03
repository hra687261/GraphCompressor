#include "compAdjList.h"

int main(int argc, char **argv)
{

  if (argc != 3){
    fprintf(stderr, 
      "One or more arguments are not correct.\n"
      "Expected: <CAL_filepath> <bool: print return>\n");
    return EXIT_FAILURE;
  }
  uint8_t n_arg = 1;
  char *path = argv[n_arg++];
  
  time_t t1, t2, t3;

  t1 = time(NULL);
  CompAdjList *cal = read_CompAdjList(path);
  t2 = time(NULL);

  fprintf(stdout,
    "Read CompAdjList: \n"
    "    Number of nodes = %lu\n"
    "    Number of links = %lu\n"
    "    size(cd) = %lu B\n"
    "    size(adj) = %lu B\n"
    "    sizesum = %lu B\n",
    cal->n, cal->e, cal->cds * 8, cal->nbb, cal->cds * 8 + cal->nbb);

  fprintf(stdout,
    "Reading execution time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  fprintf(stdout, "Type 0 to exit.\nType 1 followed by a number < %lu,"
    " to run the breadth first algorithm starting from that node.\n",
    cal->n+1);
  
  uint8_t ch;
  uint64_t st_node;
  uint8_t p = atoi(argv[n_arg++]);

  while( scanf("%hhu", &ch) == 1 && ch != 0){
    if(ch > 1){
      fprintf(stderr, "Type 0 or 1\n");
      continue;
    }
    while(scanf("%lu", &st_node) == 1 && st_node > cal->n){
      fprintf(stderr, "Type a number < %lu.\n", cal->n+1);
    }
    uint64_t  nbv, *bfs_res;
    
    t2 = time(NULL);
    bfs_res = bfs_CompAdjList(cal, st_node, & nbv);
    t3 = time(NULL);
    fprintf(stdout,

      "BFS starting node: %lu\n    execution time = %ldh%ldm%lds\n", st_node,
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    if (p) {
      fprintf(stdout, "    res: ");
      for (uint64_t i = 0; i < nbv; i++)
        fprintf(stdout, "%lu ", bfs_res[i]);
      fprintf(stdout, "\n");
    }
    fprintf(stdout, "    length: %lu\n", nbv);
    free(bfs_res);
  }
  free_CompAdjList(cal);
  t2 = time(NULL);
  fprintf(stderr,
    "Overall time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  return EXIT_SUCCESS;
}