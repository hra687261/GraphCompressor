#include "compAdjList.h"

int main(int argc, char **argv)
{
  if (argc < 4){
    fprintf(stderr, "Missing args! \nExpected: <CAL_filepath>"
    " <pagerank nb_iterations> <alpha> <list of nodes to print>\n");
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

  uint64_t nb_it = atoi(argv[n_arg++]);
  double alpha = atof(argv[n_arg++]);

  t2 = time(NULL);
  double *pagerank=power_iteration(cal,nb_it,alpha);
  t3 = time(NULL);
  
  fprintf(stdout,
    "PageRank execution time = %ldh%ldm%lds\n",
    (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));
  
  fprintf(stdout, "PageRank result:\n");

  while(n_arg < argc){
    uint64_t pnode = atoi(argv[n_arg++]);
    fprintf(stdout,"    PageRank[%lu] = %.20lf\n", pnode, pagerank[pnode]);
  } 

  uint64_t mpr=0.;
  for(uint64_t i=0; i< cal->n+1;i++)
    if(pagerank[i]>pagerank[mpr])
      mpr=i;
  fprintf(stdout,"The (first) Node with the highest PageRank value %lu:"
    "\n    PageRank[%lu] = %.20lf\n", mpr, mpr, pagerank[mpr]);

  free(pagerank);
  free_CompAdjList(cal);
  
  t2 = time(NULL);
  fprintf(stdout,
    "Overall time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  return EXIT_SUCCESS;
}