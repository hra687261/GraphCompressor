#include "compAdjListG.h"
#include "compAdjListCL.h"
#include "compAdjListCB.h"
#include "compAdjListI.h"
#include "compAdjList.h"

int main(int argc , char **argv)
{ 
  assert( (argc>=6 && argc<=11) && 
"expected args : <datastrutcture_id> <edgelist_filepath> <nb_lines_to_ignore> \
<output_filepath> <function_id{1..4}> [<k_shrinking_parameter> \
<window_size> <max_ref_count> <threshold> <jump-len>] ");

char *path=argv[2], 
  *out=argv[4];
      
uint8_t ds_id=atoi(argv[1]),
  ign=atoi(argv[3]),
  func_id=atoi(argv[5]),
  k=1, n_arg=6;

  uint64_t u=0;
  if(func_id==4) {
    k=atoi(argv[n_arg++]);
    setK(k);
  }
   
  uint64_t ws=atoi(argv[n_arg++]),
    mrc=atoi(argv[n_arg++]),
    threshold=atoi(argv[n_arg++]),
    jump=atoi(argv[n_arg++]); 
  time_t t1, t2, t3;


  fprintf(stderr,">>> ds_id : %u\n>>> elfp : %s\n>>> ign : %u \n>>> outfp : %s\n\
  >>> func_id=%u \n>>> k=%u \n>>> ws=%lu \n>>> mrc=%lu \n>>> th=%lu \n>>> jump=%lu\n",
  ds_id, path, ign, out, func_id, k, ws, mrc, threshold, jump);


  if (ds_id == 1){
  //CompAdjListG
    {
      t1=time(NULL);
      CompAdjListG *al=load_CompAdjListG(path, ign, 
                        get_encoding_function(func_id),
                        get_expl_function(func_id), 
                        &u);
      fprintf(stderr,
        "adjlg : n=(%lu)  e=(%lu)  cd=(%lu)  adj=(%lu)  sum=(%lu)\n", 
        al->n, al->e, (al->n+2)*8, u/8 + 1, (al->n+2)*8 + u/8 + 1); 
      
      t2=time(NULL);
	    fprintf(stderr,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2-t1)/3600, ((t2-t1)%3600)/60, ((t2-t1)%60));
      
      t2=time(NULL);
      write_CompAdjListG(al,func_id,out);
      
      t3=time(NULL);
	    fprintf(stderr,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3-t2)/3600,((t3-t2)%3600)/60,((t3-t2)%60));


      free_CompAdjListG(al);
      t2=time(NULL);
	    fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2-t1)/3600,((t2-t1)%3600)/60,((t2-t1)%60));
    }
  } else if (ds_id == 2){
  //CompAdjListCL
    {
      t1=time(NULL);
      CompAdjListCL *al=load_CompAdjListCL(path, ign, 
                          get_encoding_function(func_id),
                          get_expl_function(func_id),
                          &u, ws, mrc);
      fprintf(stderr,
        "adjlcl : n=(%lu)  e=(%lu)  cd=(%lu)  adj=(%lu)  sum=(%lu)\n", 
        al->n, al->e, (al->n+2)*8, u/8 + 1, (al->n+2)*8 + u/8 + 1); 
      
      t2=time(NULL);
	    fprintf(stderr,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2-t1)/3600, ((t2-t1)%3600)/60, ((t2-t1)%60));
      
      t2=time(NULL);
      write_CompAdjListCL(al,func_id,out);
      
      t3=time(NULL);
	    fprintf(stderr,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3-t2)/3600,((t3-t2)%3600)/60,((t3-t2)%60));

      free_CompAdjListCL(al);
      t2=time(NULL);
	    fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2-t1)/3600,((t2-t1)%3600)/60,((t2-t1)%60));
    }
  } else if (ds_id == 3){
  //CompAdjListCB
    {
      t1=time(NULL);
      //fprintf(stderr,"ici %s %u %u  %lu %lu %lu\n",path,ign,func_id, u,ws,mrc);
      CompAdjListCB *al=load_CompAdjListCB(path,ign,
                          get_encoding_function(func_id),
                          get_expl_function(func_id),
                          &u, ws, mrc);
      fprintf(stderr,
        "adjlcb : n=(%lu)  e=(%lu)  cd=(%lu)  adj=(%lu)  sum=(%lu)\n", 
        al->n, al->e, (al->n+2)*8, u/8 + 1, (al->n+2)*8 + u/8 + 1); 
      
      t2=time(NULL);
	    fprintf(stderr,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2-t1)/3600, ((t2-t1)%3600)/60, ((t2-t1)%60));
      
      t2=time(NULL);
      write_CompAdjListCB(al,func_id,out);
      
      t3=time(NULL);
	    fprintf(stderr,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3-t2)/3600,((t3-t2)%3600)/60,((t3-t2)%60));

      free_CompAdjListCB(al);
      t2=time(NULL);
	    fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2-t1)/3600,((t2-t1)%3600)/60,((t2-t1)%60));
    }
  } else if (ds_id == 4){
  //CompAdjListI
    {
      t1=time(NULL);
      CompAdjListI *al=load_CompAdjListI(path, ign,
                        get_encoding_function(func_id),
                        get_expl_function(func_id),
                        &u, ws, mrc, threshold);
      fprintf(stderr,
        "adjli : n=(%lu)  e=(%lu)  cd=(%lu)  adj=(%lu)  sum=(%lu)\n", 
        al->n, al->e, (al->n+2)*8, u/8 + 1, (al->n+2)*8 + u/8 + 1); 
      
      t2=time(NULL);
	    fprintf(stderr,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2-t1)/3600, ((t2-t1)%3600)/60, ((t2-t1)%60));
      
      t2=time(NULL);
      write_CompAdjListI(al,func_id,out);
      
      t3=time(NULL);
	    fprintf(stderr,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3-t2)/3600,((t3-t2)%3600)/60,((t3-t2)%60));

      free_CompAdjListI(al);
      t2=time(NULL);
	    fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2-t1)/3600,((t2-t1)%3600)/60,((t2-t1)%60));    
    }
  } else if (ds_id == 5){
  //CompAdjList
    {
      t1=time(NULL);
      CompAdjList *al=load_CompAdjList(path, ign,
                        get_encoding_function(func_id),
                        get_expl_function(func_id),
                        &u, jump, ws, mrc, threshold);
      fprintf(stderr,
        "adjliv2 : n=(%lu) e=(%lu) cd=(%lu %lu)  adj=(%lu)  sum=(%lu)\n", 
        al->n, al->e, ((al->n-1)/al->j+1)*8, al->cds*8, al->nbb, ((al->n-1)/al->j+1)*8 + al->nbb); 
      
      t2=time(NULL);
	    fprintf(stderr,
        "Reading execution time = %ldh%ldm%lds\n",
        (t2-t1)/3600, ((t2-t1)%3600)/60, ((t2-t1)%60));
      
      t2=time(NULL);
      write_CompAdjList(al,func_id,out);
      
      t3=time(NULL);
	    fprintf(stderr,
        "Writing execution time = %ldh%ldm%lds\n",
        (t3-t2)/3600,((t3-t2)%3600)/60,((t3-t2)%60));

      free_CompAdjList(al);
      t2=time(NULL);
	    fprintf(stderr,
        "Overall time = %ldh%ldm%lds\n",
        (t2-t1)/3600,((t2-t1)%3600)/60,((t2-t1)%60));    
    }
  }
  
  return EXIT_SUCCESS;
}


