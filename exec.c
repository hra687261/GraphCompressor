
#include "compAdjListG.h"
#include "compAdjListCL.h"
#include "compAdjListCB.h"
#include "compAdjListI.h"
#include "compAdjList.h" 
#include "integerEncoding.h"
#include "exp.h"

int main(int argc __attribute__((unused)), 
  char **argv __attribute__((unused)))
{ 
  uint8_t id;
  uint64_t *b,nbv;
  uint64_t nbvals=15,timesum, avtile_u64;

  CompAdjList *al=read_CompAdjList(argv[1],&id);


  fprintf(stderr,
    "load : n=(%lu)  e=(%lu)  cd=(%lu)  adj=(%lu)  sum=(%lu)  id=%u  k=%u\n", 
    al->n, al->e, (al->n+1)*8, al->nbb, (al->n+1)*8 +al->nbb, id, getK());

  uint64_t *t=uniform_dist_ints(nbvals,al->n);
  time_t t1, t2;

  timesum=0;
  for(uint64_t i=0;i<nbvals;i++){
    t1=time(NULL);
    nbv=0;
    b=bfs_CompAdjList(al,t[i],&nbv,get_decoding_function(id));
    free(b);
    t2=time(NULL);
    timesum+=t2-t1;
  }

  avtile_u64=timesum/nbvals;
  fprintf(stderr,"Average random access to successor list time : %luh%lum%lus  %lu\n",
       avtile_u64/3600, avtile_u64%3600/60, avtile_u64%60, avtile_u64);

  t1=time(NULL);
  power_iteration(al,1,0.15,get_decoding_function(id));
  t2=time(NULL);
  timesum=t2-t1;
  
  fprintf(stderr,"PageRank it time: %luh%lum%lus  %lu\n",
       timesum/3600, timesum%3600/60, timesum%60, timesum);

  free_CompAdjList(al);
  free(t);
  return EXIT_SUCCESS;
}    
  
