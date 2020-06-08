#include "compAdjListG.h"
#include "compAdjListCL.h"
#include "compAdjListCB.h"
#include "compAdjListI.h"

int main(int argc, char **argv)
{
  if (argc != 5) {
    fprintf(stderr,
      "Missing args! \nexpected: <datastructure_id{0..3}> <CAL_filepath>\
 <bfs-starting-node> <bool: print bfs return>\n");
    return EXIT_FAILURE;
  }

  char *path = argv[2];
  uint8_t ds_id = atoi(argv[1]),
    p = atoi(argv[4]), id;
  uint64_t firstn = atoi(argv[3]), nbv, * bfs_res;

  time_t t1, t2, t3;
  fprintf(stderr, ">>> ds_id: %u\n>>> cmpfilepath: %s\n\
>>> bfs_s_n: %lu \n>>> p_bfs: %u\n\n", ds_id, path, firstn, p);

  if (ds_id == 0) {
    //CompAdjListG
    t1 = time(NULL);
    CompAdjListG *al = read_CompAdjListG(path, & id);

    t2 = time(NULL);

    fprintf(stderr,
      "Read CompAdjListG: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
      al->n, al->e, (al->n + 1) * 8, al->nbb, (al->n + 1) * 8 + al->nbb);

    fprintf(stderr,
      "Reading execution time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

    t2 = time(NULL);
    bfs_res = bfs_CompAdjListG(al, firstn, &nbv, get_decoding_function(id));
    t3 = time(NULL);

    fprintf(stderr,
      "BFS execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    if (p) {
      for (uint64_t i = 0; i < nbv; i++)
        fprintf(stderr, "%lu ", bfs_res[i]);
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "BFS length : %lu\n", nbv);

    t2 = time(NULL);
    fprintf(stderr,
      "Overall time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

    free(bfs_res);
    free_CompAdjListG(al);

  } else if (ds_id == 1) {
    //CompAdjListCL

    t1 = time(NULL);
    CompAdjListCL *al = read_CompAdjListCL(path, & id);

    t2 = time(NULL);

    fprintf(stderr,
      "Read CompAdjListCL: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
      al->n, al->e, (al->n + 1) * 8, al->nbb, (al->n + 1) * 8 + al->nbb);

    fprintf(stderr,
      "Reading execution time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

    t2 = time(NULL);
    bfs_res = bfs_CompAdjListCL(al, firstn, & nbv, get_decoding_function(id));

    t3 = time(NULL);
    fprintf(stderr,
      "BFS execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    //affichage du rendu du parcours en largeur
    if (p) {
      for (uint64_t i = 0; i < nbv; i++)
        fprintf(stderr, "%lu ", bfs_res[i]);
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "BFS length : %lu\n", nbv);

    t2 = time(NULL);
    fprintf(stderr,
      "Overall time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    free(bfs_res);
    free_CompAdjListCL(al);

  } else if (ds_id == 2) {
    //CompAdjListCB

    t1 = time(NULL);
    CompAdjListCB *al = read_CompAdjListCB(path, & id);

    t2 = time(NULL);

    fprintf(stderr,
      "Read CompAdjListCB: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
      al->n, al->e, (al->n + 1) * 8, al->nbb, (al->n + 1) * 8 + al->nbb);

    fprintf(stderr,
      "Reading execution time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

    t2 = time(NULL);
    bfs_res = bfs_CompAdjListCB(al, firstn, &nbv, get_decoding_function(id));

    t3 = time(NULL);
    fprintf(stderr,
      "BFS execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    if (p) {
      for (uint64_t i = 0; i < nbv; i++)
        fprintf(stderr, "%lu ", bfs_res[i]);
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "BFS length : %lu\n", nbv);

    t2 = time(NULL);
    fprintf(stderr,
      "Overall time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    free(bfs_res);
    free_CompAdjListCB(al);

  } else if (ds_id == 3) {
    //CompAdjListI

    t1 = time(NULL);
    CompAdjListI *al = read_CompAdjListI(path, & id);

    t2 = time(NULL);

    fprintf(stderr,
      "Read CompAdjListI: \n    Number of nodes = %lu\n\
    Number of links = %lu\n    size(cd) = %lu B\n\
    size(adj) = %lu B\n    sizesum = %lu B\n",
      al->n, al->e, (al->n + 1) * 8, al->nbb, (al->n + 1) * 8 + al->nbb);

    fprintf(stderr,
      "Reading execution time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

    t2 = time(NULL);
    bfs_res = bfs_CompAdjListI(al, firstn, &nbv, get_decoding_function(id));

    t3 = time(NULL);
    fprintf(stderr,
      "BFS execution time = %ldh%ldm%lds\n",
      (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

    if (p) {
      for (uint64_t i = 0; i < nbv; i++)
        fprintf(stderr, "%lu ", bfs_res[i]);
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "BFS length : %lu\n", nbv);

    t2 = time(NULL);
    fprintf(stderr,
      "Overall time = %ldh%ldm%lds\n",
      (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    free(bfs_res);
    free_CompAdjListI(al);
  }
  return EXIT_SUCCESS;
}



