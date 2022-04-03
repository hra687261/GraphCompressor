#include "compAdjListG.h"
#include "compAdjListCL.h"
#include "compAdjListCB.h"
#include "compAdjListI.h"

int main(int argc, char **argv)
{
  if (argc != 3){
    fprintf(stderr, 
      "One or more arguments are not correct.\n"
      "Expected: <CAL_filepath> <bool: print return>\n");
    return EXIT_FAILURE;
  }

  uint8_t n_arg = 1, ds_id;
  char *path = argv[n_arg++];

  fprintf(stderr,
    "Choose a compressed data structre:\n"
    "  Type 0 for the CompAdjListG. (Compressed Adjacency list with the gaps technique)\n"
    "  Type 1 for the CompAdjListCL. (Compressed Adjacency list with the copy list technique and gaps for the residuals)\n"
    "  Type 2 for the CompAdjListCB. (Compressed Adjacency list with the copy blocks technique and gaps for the residualse)\n"
    "  Type 3 for the CompAdjListI. (Compressed Adjacency list with the copy blocks and intervals technique and gaps for the residualse)\n");

  while(scanf("%hhu", &ds_id) != 1 || ds_id > 3)
    fprintf(stderr, "Choose a compressed data structure: ");

  time_t t1, t2, t3;
  if (ds_id == 0) {
    // CompAdjListG
    {
      t1 = time(NULL);
      CompAdjListG *calg = read_CompAdjListG(path);
      t2 = time(NULL);
      
      fprintf(stdout,
        "Read CompAdjListG: \n"
        "    Number of nodes = %lu\n"
        "    Number of links = %lu\n"
        "    size(cd) = %lu B\n"
        "    size(adj) = %lu B\n"
        "    sizesum = %lu B\n",
        calg->n, calg->e, (calg->n+2) * 8, calg->nbb, (calg->n+2) * 8 + calg->nbb);

      fprintf(stdout,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

      fprintf(stdout, "Type 0 to exit.\nType 1 followed by a number < %lu,"
        " to run the breadth first algorithm starting from that node.\n",
        calg->n+1);
    
      uint8_t ch;
      uint64_t st_node;
      uint8_t p = atoi(argv[n_arg++]);

      while( scanf("%hhu", &ch) == 1 && ch != 0){
        if(ch > 1){
          fprintf(stderr, "Type 0 or 1\n");
          continue;
        }
        while(scanf("%lu", &st_node) == 1 && st_node > calg->n){
          fprintf(stderr, "Type a number < %lu.\n", calg->n+1);
        }
        uint64_t  nbv, *bfs_res;
        
        t2 = time(NULL);
        bfs_res = bfs_CompAdjListG(calg, st_node, & nbv);
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
      free_CompAdjListG(calg);
      t2 = time(NULL);
      fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    }
  } else if (ds_id == 1) {
    // CompAdjListCL
    {
      t1 = time(NULL);
      CompAdjListCL *calcl = read_CompAdjListCL(path);
      t2 = time(NULL);
      
      fprintf(stdout,
        "Read CompAdjListCL: \n"
        "    Number of nodes = %lu\n"
        "    Number of links = %lu\n"
        "    size(cd) = %lu B\n"
        "    size(adj) = %lu B\n"
        "    sizesum = %lu B\n",
        calcl->n, calcl->e, (calcl->n+2) * 8, calcl->nbb, (calcl->n+2) * 8 + calcl->nbb);

      fprintf(stdout,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

      fprintf(stdout, "Type 0 to exit.\nType 1 followed by a number < %lu,"
        " to run the breadth first algorithm starting from that node.\n",
        calcl->n+1);
    
      uint8_t ch;
      uint64_t st_node;
      uint8_t p = atoi(argv[n_arg++]);

      while( scanf("%hhu", &ch) == 1 && ch != 0){
        if(ch > 1){
          fprintf(stderr, "Type 0 or 1\n");
          continue;
        }
        while(scanf("%lu", &st_node) == 1 && st_node > calcl->n){
          fprintf(stderr, "Type a number < %lu.\n", calcl->n+1);
        }
        uint64_t  nbv, *bfs_res;
        
        t2 = time(NULL);
        bfs_res = bfs_CompAdjListCL(calcl, st_node, & nbv);
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
      free_CompAdjListCL(calcl);
      t2 = time(NULL);
      fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    }
  } else if (ds_id == 2) {
    // CompAdjListCB
    {
      t1 = time(NULL);
      CompAdjListCB *calcb = read_CompAdjListCB(path);
      t2 = time(NULL);
      
      fprintf(stdout,
        "Read CompAdjListCB: \n"
        "    Number of nodes = %lu\n"
        "    Number of links = %lu\n"
        "    size(cd) = %lu B\n"
        "    size(adj) = %lu B\n"
        "    sizesum = %lu B\n",
        calcb->n, calcb->e, (calcb->n+2) * 8, calcb->nbb, (calcb->n+2) * 8 + calcb->nbb);

      fprintf(stdout,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

      fprintf(stdout, "Type 0 to exit.\nType 1 followed by a number < %lu,"
        " to run the breadth first algorithm starting from that node.\n",
        calcb->n+1);
    
      uint8_t ch;
      uint64_t st_node;
      uint8_t p = atoi(argv[n_arg++]);

      while( scanf("%hhu", &ch) == 1 && ch != 0){
        if(ch > 1){
          fprintf(stderr, "Type 0 or 1\n");
          continue;
        }
        while(scanf("%lu", &st_node) == 1 && st_node > calcb->n){
          fprintf(stderr, "Type a number < %lu.\n", calcb->n+1);
        }
        uint64_t  nbv, *bfs_res;
        
        t2 = time(NULL);
        bfs_res = bfs_CompAdjListCB(calcb, st_node, & nbv);
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
      free_CompAdjListCB(calcb);
      t2 = time(NULL);
      fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    }
  } else if (ds_id == 3) {
    // CompAdjListI
    {
      t1 = time(NULL);
      CompAdjListI *cali = read_CompAdjListI(path);
      t2 = time(NULL);
      
      fprintf(stdout,
        "Read CompAdjListI: \n"
        "    Number of nodes = %lu\n"
        "    Number of links = %lu\n"
        "    size(cd) = %lu B\n"
        "    size(adj) = %lu B\n"
        "    sizesum = %lu B\n",
        cali->n, cali->e, (cali->n+2) * 8, cali->nbb, (cali->n+2) * 8 + cali->nbb);

      fprintf(stdout,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

      fprintf(stdout, "Type 0 to exit.\nType 1 followed by a number < %lu,"
        " to run the breadth first algorithm starting from that node.\n",
        cali->n+1);
    
      uint8_t ch;
      uint64_t st_node;
      uint8_t p = atoi(argv[n_arg++]);

      while( scanf("%hhu", &ch) == 1 && ch != 0){
        if(ch > 1){
          fprintf(stderr, "Type 0 or 1\n");
          continue;
        }
        while(scanf("%lu", &st_node) == 1 && st_node > cali->n){
          fprintf(stderr, "Type a number < %lu.\n", cali->n+1);
        }
        uint64_t  nbv, *bfs_res;
        
        t2 = time(NULL);
        bfs_res = bfs_CompAdjListI(cali, st_node, & nbv);
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
      free_CompAdjListI(cali);
      t2 = time(NULL);
      fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    }
  }
  return EXIT_SUCCESS;
}



