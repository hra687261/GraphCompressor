#include "compAdjList.h"

int main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr,
      "Missing args! \nexpected: <CAL_filepath>\
 <algorithm{1 or 2}> <bool: print return> [<bfs-starting-node>|[<pagerank nb_iterations> <alpha> <list of nodes to print>]] \n");
    return EXIT_FAILURE;
  }
  char *path = argv[1];
  uint8_t id,
   algo = atoi(argv[2]),
   p = atoi(argv[3]);

  

  time_t t1, t2, t3;
  fprintf(stderr, ">>> cmpfilepath : %s\n\
>>> p_res : %u\n\n", path, p);

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




  if ( algo ==  1 ) {
    if ( argc != 5 ) {
      fprintf(stderr,
      "Missing argument: <bfs-starting-node>!\n");
      return EXIT_FAILURE;
    }

    uint64_t  nbv, *bfs_res,
      bfsfn=atoi(argv[4]);
    
    t2 = time(NULL);
    bfs_res = bfs_CompAdjList(cal, bfsfn, & nbv, get_decoding_function(id));
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
  
  } else if  ( algo == 2 ) {

    if ( argc < 6 ) {
      fprintf(stderr,
      "Missing argument(s): <pagerank nb_iterations> <alpha>[<list of nodes to print>]!\n");
      return EXIT_FAILURE;
    }

    uint64_t nb_it = atoi(argv[4]);
    int nb_args=6;
    double alpha = atof(argv[5]);
    
    t2 = time(NULL);
    double *pagerank=power_iteration(cal,nb_it,alpha,get_decoding_function(id));
    t3 = time(NULL);
    fprintf(stderr,
      "PageRank execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    if(p){
      fprintf(stderr, "PageRank res: ");
      if( nb_args < argc ) {
        fprintf(stderr,"\n");
        while ( nb_args < argc ) {
          uint64_t pnode = atoi(argv[nb_args++]);
          fprintf(stderr,">>>> pr[%lu] = %.20lf\n", pnode, pagerank[pnode]);
        }
      } 
      fprintf(stderr, "\n");
    }

    uint64_t mpr=0.;
    for(uint64_t i=0; i< cal->n+1;i++)
      if(pagerank[i]>pagerank[mpr])
        mpr=i;
    fprintf(stderr,"Node with the highest PageRank %lu: pr[%lu] = %.20lf\n", mpr, mpr, pagerank[mpr]);

    free(pagerank);
  }

  free_CompAdjList(cal);
  t2 = time(NULL);
  fprintf(stderr,
    "Overall time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  return EXIT_SUCCESS;
}