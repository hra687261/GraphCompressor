#include "compAdjList.h"

int main(int argc, char ** argv)
{
  if (argc != 7) {
    fprintf(stderr,
      "One or more arguments are not correct.\n"
      "Expected: <edgelist_filepath> <output_filepath> <window_size(>0)>" 
      "<max_ref_count(>0)> <threshold(>=2)> <jump-len(>0)>\n");
    return EXIT_FAILURE;
  }

  uint8_t n_arg = 1;
  char *path = argv[n_arg++],
    *output = argv[n_arg++];
  uint64_t u = 0,
    ws = atoi(argv[n_arg++]),
    mrc = atoi(argv[n_arg++]),
    threshold = atoi(argv[n_arg++]),
    jump = atoi(argv[n_arg++]);

  fprintf(stderr,
    "Choose the compression codes:\n"
    "  Type 0 for the unary code.\n"
    "  Type 1 for the gamma code.\n"
    "  Type 2 for the delta code.\n"
    "  Type 3 for the nibble code.\n"
    "  Type 4 for the zeta-k code, followed by a the value of k (>0).\n");
  char *secs[NB_SECTIONS]=
  { "number of used bits by the compressed successors lists",
    "references", "block counts", "copy blocks",
    "intervals", "residuals"};

  CompCodes *ccs=calloc(1,sizeof(CompCodes));
  char *str=calloc(1000,sizeof(char));
  for(uint8_t i=0; i<NB_SECTIONS; i++){
    sprintf(str,"\nFor the %s : ",secs[i]);
    fprintf(stderr, "%s", str);
    while (scanf("%hhu", &ccs->ccs_array[i*2]) != 1 || ccs->ccs_array[i*2] > 4)
      fprintf(stderr, "Type a value between 0 and 4 to choose on of the codes : ");
    if (ccs->ccs_array[i*2] == ZETA_K_FUNC_ID)
      do{
        fprintf(stderr, "  Type a value that is > 0 for the shrinking parameter of the zeta code (k) : ");
      }while (scanf("%hhu", &ccs->ccs_array[i*2+1]) != 1 || ccs->ccs_array[i*2 + 1] == 0);
  }
  free(str);
  
  time_t t1, t2, t3;

  t1 = time(NULL);
  CompAdjList * cal = load_CompAdjList(path,ccs,&u,jump,ws,mrc,threshold);
  fprintf(stdout,
    "Loaded CompAdjList: \n"
    "    Number of nodes = %lu\n"
    "    Number of links = %lu\n"
    "    size(cd) = %lu B\n"
    "    size(adj) = %lu B\n"
    "    sizesum = %lu B\n",
    cal->n, cal->e, cal->cds * 8, cal->nbb, cal->cds * 8 + cal->nbb);

  t2 = time(NULL);
  fprintf(stdout,
    "Loading execution time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

  write_CompAdjList(cal, output);

  t3 = time(NULL);
  fprintf(stdout,
    "Writing execution time = %ldh%ldm%lds\n",
    (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

  free_CompAdjList(cal);
  t2 = time(NULL);
  fprintf(stderr,
    "Overall time = %ldh%ldm%lds\n",
    (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
  
  return EXIT_SUCCESS;
}