#include "compAdjListG.h"
#include "compAdjListCL.h"
#include "compAdjListCB.h"
#include "compAdjListI.h"

int main(int argc, char **argv)
{
  if (argc != 3){
    fprintf(stderr,
      "One or more arguments are not correct.\n"
      "Expected: <edgelist_filepath> <output_filepath>\n");
    return EXIT_FAILURE;
  }

  uint8_t n_arg = 1, ds_id;
  char *path = argv[n_arg++],
    *output = argv[n_arg++];

  fprintf(stderr,
    "Choose a compressed data structre:\n"
    "  Type 0 for the CompAdjListG. (Compressed Adjacency list with the gaps technique)\n"
    "  Type 1 for the CompAdjListCL. (Compressed Adjacency list with the copy list technique and gaps for the residuals)\n"
    "  Type 2 for the CompAdjListCB. (Compressed Adjacency list with the copy blocks technique and gaps for the residualse)\n"
    "  Type 3 for the CompAdjListI. (Compressed Adjacency list with the copy blocks and intervals technique and gaps for the residualse)\n");

  while(scanf("%hhu", &ds_id) != 1 || ds_id >3)
    fprintf(stderr, "Choose a compressed data structure: ");

  if (ds_id == 0) {
    // CompAdjListG
    {
      uint64_t u = 0;
      
      fprintf(stderr,
        "\nChoose a compression code:\n"
        "  Type 0 for the unary code.\n"
        "  Type 1 for the gamma code.\n"
        "  Type 2 for the delta code.\n"
        "  Type 3 for the nibble code.\n"
        "  Type 4 for the zeta-k code, followed by a the value of k (>0).\n");

      time_t t1, t2, t3;
      uint8_t f_id, k;
      
      while (scanf("%hhu", &f_id) != 1 || f_id > 4)
          fprintf(stderr, "Type a value between 0 and 4 to choose a code: ");
      if (f_id == ZETA_K_FUNC_ID)
        do{
          fprintf(stderr, "  Type a value that is > 0 for the shrinking parameter of the zeta code (k): ");
        }while (scanf("%hhu", &k) != 1 || k == 0);
      
      t1 = time(NULL);
      CompAdjListG * calg = load_CompAdjListG(path, f_id, k, &u);


      fprintf(stdout,
        "Loaded CompAdjListG: \n"
        "    Number of nodes = %lu\n"
        "    Number of links = %lu\n"
        "    size(cd) = %lu B\n"
        "    size(adj) = %lu B\n"
        "    sizesum = %lu B\n",
        calg->n, calg->e, (calg->n+2) * 8, calg->nbb, (calg->n+2) * 8 + calg->nbb);

      t2 = time(NULL);
      fprintf(stdout,
        "Loading execution time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

      write_CompAdjListG(calg, output);

      t3 = time(NULL);
      fprintf(stdout,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

      free_CompAdjListG(calg);
      t2 = time(NULL);
      fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));  
    }
  } else if (ds_id == 1) {
    // CompAdjListCL
    {
      uint64_t u = 0, ws, mrc;
      
      do{
        fprintf(stderr,"Type a value that is > 0 for the window size: ");
      }while (scanf("%lu",&ws) != 1 || ws <= 0 );

      do{
        fprintf(stderr,"Type a value that is > 0 for the max ref count: ");
      }while (scanf("%lu",&mrc) != 1 || mrc <= 0 );

      fprintf(stderr,
        "\nChoose the compression codes:\n"
        "  Type 0 for the unary code.\n"
        "  Type 1 for the gamma code.\n"
        "  Type 2 for the delta code.\n"
        "  Type 3 for the nibble code.\n"
        "  Type 4 for the zeta-k code, followed by a the value of k (>0).\n");
      char *secs[NB_SECTIONS_CB]=
      { "references", "block counts", "copy blocks", "residuals"};
      
      CompCodes_CL *ccscl=calloc(1,sizeof(CompCodes_CL));

      char *str=calloc(1000,sizeof(char));
      for(uint8_t i=0; i<NB_SECTIONS_CL; i++){
        sprintf(str,"\nFor the %s: ",secs[i]);
        fprintf(stderr, "%s", str);
        while (scanf("%hhu", &ccscl->ccscl_array[i*2]) != 1 || ccscl->ccscl_array[i*2] > 4)
          fprintf(stderr, "Type a value between 0 and 4 to choose on of the codes: ");
        if (ccscl->ccscl_array[i*2] == ZETA_K_FUNC_ID)
          do{
            fprintf(stderr, "  Type a value that is > 0 for the shrinking parameter of the zeta code (k): ");
          }while (scanf("%hhu", &ccscl->ccscl_array[i*2+1]) != 1 || ccscl->ccscl_array[i*2 + 1] == 0);
      }
      free(str);

      time_t t1, t2, t3;

      t1 = time(NULL);
      CompAdjListCL * calcl = load_CompAdjListCL(path,ccscl,&u,ws,mrc);

      fprintf(stdout,
        "Loaded CompAdjListCL: \n"
        "    Number of nodes = %lu\n"
        "    Number of links = %lu\n"
        "    size(cd) = %lu B\n"
        "    size(adj) = %lu B\n"
        "    sizesum = %lu B\n",
        calcl->n, calcl->e, (calcl->n+2) * 8, calcl->nbb, (calcl->n+2) * 8 + calcl->nbb);

      t2 = time(NULL);
      fprintf(stdout,
        "Loading execution time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

      write_CompAdjListCL(calcl, output);

      t3 = time(NULL);
      fprintf(stdout,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

      free_CompAdjListCL(calcl);
      t2 = time(NULL);
      fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    }
  } else if (ds_id == 2) {
    // CompAdjListCB
    {
      uint64_t u = 0, ws, mrc;
      
      do{
        fprintf(stderr,"Type a value that is > 0 for the window size: ");
      }while (scanf("%lu",&ws) != 1 || ws <= 0 );

      do{
        fprintf(stderr,"Type a value that is > 0 for the max ref count: ");
      }while (scanf("%lu",&mrc) != 1 || mrc <= 0 );

      fprintf(stderr,
        "\nChoose the compression codes:\n"
        "  Type 0 for the unary code.\n"
        "  Type 1 for the gamma code.\n"
        "  Type 2 for the delta code.\n"
        "  Type 3 for the nibble code.\n"
        "  Type 4 for the zeta-k code, followed by a the value of k (>0).\n");
      char *secs[NB_SECTIONS_CB]=
      { "references", "block counts", "copy blocks", "residuals"};
      
      CompCodes_CB *ccscb=calloc(1,sizeof(CompCodes_CB));

      char *str=calloc(1000,sizeof(char));
      for(uint8_t i=0; i<NB_SECTIONS_CB; i++){
        sprintf(str,"\nFor the %s: ",secs[i]);
        fprintf(stderr, "%s", str);
        while (scanf("%hhu", &ccscb->ccscb_array[i*2]) != 1 || ccscb->ccscb_array[i*2] > 4)
          fprintf(stderr, "Type a value between 0 and 4 to choose on of the codes: ");
        if (ccscb->ccscb_array[i*2] == ZETA_K_FUNC_ID)
          do{
            fprintf(stderr, "  Type a value that is > 0 for the shrinking parameter of the zeta code (k): ");
          }while (scanf("%hhu", &ccscb->ccscb_array[i*2+1]) != 1 || ccscb->ccscb_array[i*2 + 1] == 0);
      }
      free(str);

      time_t t1, t2, t3;

      t1 = time(NULL);
      CompAdjListCB * calcb = load_CompAdjListCB(path,ccscb,&u,ws,mrc);

      fprintf(stdout,
        "Loaded CompAdjListCB: \n"
        "    Number of nodes = %lu\n"
        "    Number of links = %lu\n"
        "    size(cd) = %lu B\n"
        "    size(adj) = %lu B\n"
        "    sizesum = %lu B\n",
        calcb->n, calcb->e, (calcb->n+2) * 8, calcb->nbb, (calcb->n+2) * 8 + calcb->nbb);

      t2 = time(NULL);
      fprintf(stdout,
        "Loading execution time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

      write_CompAdjListCB(calcb, output);

      t3 = time(NULL);
      fprintf(stdout,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

      free_CompAdjListCB(calcb);
      t2 = time(NULL);
      fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    }
  } else if (ds_id == 3) {
    // CompAdjListI
    {
      uint64_t u = 0, ws, mrc, threshold;
      
      do{
        fprintf(stderr,"Type a value that is > 0 for the window size: ");
      }while (scanf("%lu",&ws) != 1 || ws <= 0 );

      do{
        fprintf(stderr,"Type a value that is > 0 for the max ref count: ");
      }while (scanf("%lu",&mrc) != 1 || mrc <= 0 );

      do{
        fprintf(stderr,"Type a value that is >= 2 for the interval threshold: ");
      }while (scanf("%lu",&threshold) != 1 || threshold < 2 );

      fprintf(stderr,
        "\nChoose the compression codes:\n"
        "  Type 0 for the unary code.\n"
        "  Type 1 for the gamma code.\n"
        "  Type 2 for the delta code.\n"
        "  Type 3 for the nibble code.\n"
        "  Type 4 for the zeta-k code, followed by a the value of k (>0).\n");
      char *secs[NB_SECTIONS_I]=
      { "number of used bits by the compressed successors lists",
        "references", "block counts", "copy blocks",
        "intervals", "residuals"};
      
      CompCodes_I *ccsi=calloc(1,sizeof(CompCodes_I));

      char *str=calloc(1000,sizeof(char));
      for(uint8_t i=0; i<NB_SECTIONS_I; i++){
        sprintf(str,"\nFor the %s: ",secs[i]);
        fprintf(stderr, "%s", str);
        while (scanf("%hhu", &ccsi->ccsi_array[i*2]) != 1 || ccsi->ccsi_array[i*2] > 4)
          fprintf(stderr, "Type a value between 0 and 4 to choose on of the codes: ");
        if (ccsi->ccsi_array[i*2] == ZETA_K_FUNC_ID)
          do{
            fprintf(stderr, "  Type a value that is > 0 for the shrinking parameter of the zeta code (k): ");
          }while (scanf("%hhu", &ccsi->ccsi_array[i*2+1]) != 1 || ccsi->ccsi_array[i*2 + 1] == 0);
      }
      free(str);

      time_t t1, t2, t3;

      t1 = time(NULL);
      CompAdjListI * cali = load_CompAdjListI(path,ccsi,&u,ws,mrc,threshold);

      fprintf(stdout,
        "Loaded CompAdjListI: \n"
        "    Number of nodes = %lu\n"
        "    Number of links = %lu\n"
        "    size(cd) = %lu B\n"
        "    size(adj) = %lu B\n"
        "    sizesum = %lu B\n",
        cali->n, cali->e, (cali->n+2) * 8, cali->nbb, (cali->n+2) * 8 + cali->nbb);

      t2 = time(NULL);
      fprintf(stdout,
        "Loading execution time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));

      write_CompAdjListI(cali, output);

      t3 = time(NULL);
      fprintf(stdout,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3 - t2) / 3600, ((t3 - t2) % 3600) / 60, ((t3 - t2) % 60));

      free_CompAdjListI(cali);
      t2 = time(NULL);
      fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60));
    }
  }
  return EXIT_SUCCESS;
}