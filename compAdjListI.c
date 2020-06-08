#include "compAdjListI.h"

/**
 * Frees the CompAdjListI passed as a parameter.
 */
void free_CompAdjListI(CompAdjListI *cali) {
  free(cali->adj);
  free(cali->cd);
  free(cali);
}



/**
 * Loads a CompAdjListI from a file containing an edgelist,
 * "path" is the path to the file, the file must contain on every line 
 * a pair of ints x y, separated by a space, in which x->y is a link,
 * the list needs to be sorted in increasing order by the first, then the 
 * second column.
 * 
 * "*u" points to first bit after the last used bit for the array of 
 * compressed successor lists.
 * ws: the window size.
 * mrc: max ref count.
 * threshold: minimum length of an interval.
 */
CompAdjListI *load_CompAdjListI(char *path,
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val),
  uint64_t expl_function(uint64_t val),
  uint64_t *u,  uint8_t ws, 
  uint8_t mrc, uint8_t threshold)
{
  uint64_t i, j;
  CompAdjListI *cali = malloc(sizeof(CompAdjListI));
  cali->n=0;
  cali->e=0;
  cali->th=threshold;
  uint64_t v1, v2, 
    // upper bound of the memory size we'll need to allocate (arbitrary)
    alloc_ub = 0,

    precv = 0, 
    max_deg = 0, 
    new_deg = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // counting the max degree, the number of nodes and the number of edges
  FILE *file = fopen(path, "r");
  while (fscanf(file, "%lu %lu", & v1, & v2) == 2) {
    if (v1 == precv) {
      new_deg++;
    } else {
      if (new_deg > max_deg)
        max_deg = new_deg;
      new_deg = 1;
      precv = v1;
    }
    cali->n = max3(cali->n, v1, v2);
    cali->e++;
    alloc_ub += expl_function(v2)*2;
  }
  cali->cd = calloc((cali->n + 2), sizeof(uint64_t));
  cali->md = max_deg; 
  cali->adj = calloc(alloc_ub / 8 + 1, sizeof(uint8_t));


  uint64_t wcalien = ws + 1,
    
    // max number of intervals 
    m_nb_intrs = (max_deg - 1)/cali->th + 1;
  
  uint64_t c_w_ind = 0, it_w_ind = 0, ref_w_ind,
    /*
    We represent the window with 4 arrays

    window_succ : 
      contains the consecutive lists of successors that are part of the window 
      + 1 the list of successors of the node we are going to compress
    window_pos :
      contains for each node of the lists of successors in window_succ it's position 
      after the compressiong the risk
      because after compressing the the list of successors the order of the nodes might be change
    window_lens : 
      contains the lengths of the lists (of successors and their positions) that are part of the window
    window_nodes :
      contains the nodes that are part of the window
    */    
    *window_succ = malloc(max_deg * wcalien * sizeof(uint64_t)),
    *window_pos = malloc(max_deg * wcalien * sizeof(uint64_t)),
    *window_lens = calloc(wcalien, sizeof(uint64_t)),
    *window_nodes = calloc(wcalien, sizeof(uint64_t)),
    
    c_node=0, it_node, 
    tmppos_2, nb_v_tocode,

    // copy list the list of copied nodes from the referenced list of successors
    *copy_list = calloc(max_deg, sizeof(uint64_t)),
    
    // intrvs[0] : nb blocks
    // intrvs[1] : if it's =0 :
      // intrvs[2] : left-ext - c_node 
    // else : 
      // intrvs[2] : c_node - left-ext 
    // intrvs[1 .. m_nb_intrs+1[ : left extremes 
    // intrvs[m_nb_intrs+1 .. m_nb_intrs*2 + 1[ : block lengths
    *intrvs=calloc(2+m_nb_intrs*2,sizeof(uint64_t));

  uint8_t     
    /*
    contains the legnth of the reference chain of each 
    ex : (15 references 12 which references 5 -> ref_cnts[5]=0, ref_cnts[12]=1, ref_cnts[15]=2  )
    */
    *ref_cnts = calloc(cali->n + 1, sizeof(uint8_t)),

    /* 
    array of 0s and 1s in which coded[x]=1 if x 
    was coded with the reference or the intervals method
    and 0 otherwise (it's used to change the positions of the nodes
    in the successors list after the compression)
    */
    *coded = calloc(max_deg, sizeof(uint8_t));

  // the number of values that were compressed with the reference technique
  uint64_t nbv_refc = 0,
  
    // the reference (c_node-ref : the referred node)
    ref = 0;

  // the last node that had it's successors list compressed
  uint64_t precun=0;

  *u=0;
  v1=0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // compressing the edge list
  rewind(file);
  for(uint64_t ind=0;ind<cali->e+1;ind++){
    if(ind < cali->e)
      assert(fscanf(file, "%lu %lu", &v1, &v2) == 2);

    if (v1 == window_nodes[c_w_ind] 
      && ind > 0 
      && ind < cali->e) 
    {
      tmppos_2 = c_w_ind * max_deg + window_lens[c_w_ind];
      window_succ[tmppos_2] = v2;
      window_pos[tmppos_2] = window_lens[c_w_ind];
      window_lens[c_w_ind]++;
    } else {
      it_w_ind = (c_w_ind + 1) % wcalien;
      c_node = window_nodes[c_w_ind];
      it_node = window_nodes[it_w_ind];

      nbv_refc = 0;
      ref = 0;

      uint64_t
        *c_arr = window_succ + max_deg * c_w_ind,
        *ca_pos = window_pos + max_deg * c_w_ind,
        ccalien = window_lens[c_w_ind];

      if(ccalien > 0){

        // choosing the reference list
        while (it_w_ind != c_w_ind &&
          ref_cnts[it_node] < mrc) {
          uint64_t
            *it_arr = window_succ + it_w_ind * max_deg,
            it_len = window_lens[it_w_ind];

          nb_v_tocode = 0;
          i = 0;
          j = 0;
          while (i < it_len && j < ccalien) 
            if (it_arr[i] == c_arr[j]) {
              nb_v_tocode++;
              i++;
              j++;
            } else if (it_arr[i] > c_arr[j]) {
              j++;
            } else {
              i++;
            }
          

          if (nb_v_tocode > nbv_refc) {
            nbv_refc = nb_v_tocode;
            ref = c_node - it_node;
            ref_w_ind = it_w_ind;
          }
          
          it_w_ind = (it_w_ind + 1) % wcalien;
          it_node = window_nodes[it_w_ind];
        }
        
        //writing the reference
        encoding_function(cali->adj, u, ref);
        
        if (ref > 0){
          //updating the ref count
          ref_cnts[c_node] = ref_cnts[window_nodes[ref_w_ind]] + 1;

          uint64_t 
            p1 = 0, p2 = nbv_refc, 
            
            ra_it = 0,
            ca_it = 0,

            // the referenced list of successors (sorted in incremental order)
            *r_arr = window_succ + max_deg * ref_w_ind,
            
            // the real positions of the nodes in the referenced list of successors after compression (if the list was compressed)
            *ra_pos = window_pos + max_deg * ref_w_ind,
            
            // the length of the referenced list 
            rcalien = window_lens[ref_w_ind];

          // building the copy_list
          while (ra_it < rcalien && ca_it < ccalien) 
            if (r_arr[ra_it] == c_arr[ca_it]) {
              ca_pos[ca_it] = p1++;
              coded[ca_pos[ca_it++]] = 1;
              copy_list[ra_pos[ra_it++]] = 1;
            } else if (r_arr[ra_it] > c_arr[ca_it]) {
              ca_pos[ca_it] = p2++;
              ca_it++;
              //coded[ca_pos[ca_it++]] = 0;
            } else {
              ra_it++;
              //copy_list[ra_pos[ra_it++]] = 0;
            }
          

          // updating the positions of the nodes in the current array (the one we're about to compress)
          uint64_t cpt=0;
          for(i=0;i<rcalien;i++){
            if(copy_list[i]){
              cpt+=copy_list[i];
              copy_list[i]=cpt;
            }
          }

          ra_it=0;
          ca_it=0;
          while (ra_it < rcalien && ca_it < ccalien)
            if (r_arr[ra_it] == c_arr[ca_it]) {   
              ca_pos[ca_it] = copy_list[ra_pos[ra_it]]-1;   
              copy_list[ra_pos[ra_it]] = 1;
              ca_it++;
              ra_it++;
            } else if (r_arr[ra_it] > c_arr[ca_it]){
              ca_it++;
            } else { 
              ra_it++;
            }
          
          // the first block is always a 1 block
          uint8_t cv=1;
          // the number copy blocks
          uint64_t nb_blocks=1;

          //counting the number of blocks
          for (i = 0; i < rcalien; i++) 
            if(copy_list[i] != cv){
              nb_blocks++;
              cv=!cv;
            }
          
          
          //writing the number of copy blocks
          encoding_function(cali->adj, u, nb_blocks);
          
          uint64_t 
            cpt0=0,
            cpt1=0; 
          /*
          number of written copyblocks
          */
          cpt=0;

          ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          //writing the copy blocks
          cv=1;
          for (i = 0; i < rcalien && cpt < nb_blocks-1; i++) {
            if(cv){
              if(copy_list[i])
                cpt1++;
              else{
                encoding_function(cali->adj, u, cpt1);
                cpt++;
                cpt0=1;
                cv=!cv;
              }
            }else{
              if(copy_list[i]){
                encoding_function(cali->adj, u, cpt0);
                cpt++;
                cpt1=1;
                cv=!cv;
              }else
                cpt0++;
            }
            copy_list[i]=0;
          }
          while(i<rcalien){
            copy_list[i]=0;
            i++;
          }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    
        uint64_t 
          c_rightext = 0, 
          c_leftext = 0, 
          prec_rightext = c_node,
          cpt = 1, v = 0,
          nbv_intrvc = 0;
        /*
        previous value in the successors list
        */
        precv=window_succ[max_deg*c_w_ind];
        i=1;
        
        // building the intervals
        intrvs[1]=0;
        c_leftext=c_arr[0];
        cpt=1;
        do{
          v=c_arr[i];
          if( precv + 1 == v 
            && coded[ca_pos[i]]==0 
            && coded[ca_pos[i-1]]==0) 
          { 
            cpt++;
            c_rightext=v;
            if(i==ccalien-1){
              if(cpt>=cali->th){
                if(c_leftext >= prec_rightext){
                  intrvs[1]=0;
                  intrvs[2+intrvs[0]]=c_leftext-prec_rightext;
                }else{
                  intrvs[1]=1;
                  intrvs[2+intrvs[0]]=prec_rightext-c_leftext;
                }
                intrvs[2+m_nb_intrs+intrvs[0]]=cpt-cali->th;
                intrvs[0]++;
                nbv_intrvc+=cpt;
                for(j=i-cpt+1;j<=i;j++){
                  coded[ca_pos[j]]=2;
                }
              }
            }
          } else {
            if(cpt>=cali->th){
              if(c_leftext >= prec_rightext){
                intrvs[1]=0;
                intrvs[2+intrvs[0]]=c_leftext-prec_rightext;
              }else{
                intrvs[1]=1;
                intrvs[2+intrvs[0]]=prec_rightext-c_leftext;
              }
              intrvs[2+m_nb_intrs+intrvs[0]]=cpt-cali->th;
              intrvs[0]++;
              nbv_intrvc+=cpt;
              for(j=i-cpt;j<i;j++){
                coded[ca_pos[j]]=2;
              }
              prec_rightext=c_rightext+2;
            }
            c_leftext=v;
            cpt=1;
          }
          precv=v;
          i++;
        }while( i < ccalien );
          
        uint64_t
          p1=nbv_refc,
          p2=nbv_refc+nbv_intrvc;

        for(i=0;i<ccalien;i++){
          if(coded[ca_pos[i]]==2) {
            ca_pos[i]=p1++;
          } else if(coded[ca_pos[i]]==0) {
            ca_pos[i]=p2++;
          } 
        }

        //writing the number of intervals
        encoding_function(cali->adj,u,intrvs[0]);
      
        if(intrvs[0] > 0){
          if(intrvs[1]){
            set_nth_bit(cali->adj,u,*u);
          }else{
            clear_nth_bit(cali->adj,u,*u);
          }

          // writing the lengths of the intervals
          for(i=0;i<intrvs[0];i++){
            encoding_function(cali->adj,u,intrvs[2+i]);
          }
          
          //writing the the left extremes
          for(i=0;i<intrvs[0];i++){
            encoding_function(cali->adj,u,intrvs[2+m_nb_intrs+i]);
          }  
        }

      // writing the leftover values, the ones that weren't compressed
      for(i=0;i<ccalien;i++){
        coded[i]=0;
        if(ca_pos[i] > nbv_refc+nbv_intrvc){
          encoding_function(cali->adj, u, 
            c_arr[i] - precv - 1);
          precv = c_arr[i];
          continue;
        }

        //the first left over node
        if(ca_pos[i] == nbv_refc+nbv_intrvc){
          precv = c_arr[i];
          encoding_function(cali->adj, u, 
            func(c_node, precv));
        }
      }
    }

    // storing the starting bit adress of the next node
    cali->cd[c_node+1]=*u;
    // if there are nodes that don't have successors, their starting bit is address is equal to the  
    // ending bit address of the last node that has it's list of successors coded
    if(precun>0 && cali->cd[c_node]==0){
      for(i=precun+1;i<=c_node;i++)
        cali->cd[i]=cali->cd[precun];
    }
    precun=c_node+1;
    c_w_ind = (c_w_ind + 1) % wcalien;
    window_nodes[c_w_ind] = v1;

    tmppos_2 = c_w_ind * max_deg;
    window_succ[tmppos_2] = v2;
    window_pos[tmppos_2] = 0;
    window_lens[c_w_ind] = 1;
    intrvs[0]=0;
    }
  }


  if(cali->n+1 > precun)
    for(i=precun;i<=cali->n+1;i++)
      cali->cd[i] = *u;
      
  cali->nbb = (*u - 1) / 8 + 1;
  cali->adj = realloc(cali->adj, cali->nbb * sizeof(uint8_t));

  fclose(file);
  free(window_succ);
  free(window_pos);
  free(window_lens);
  free(window_nodes);
  free(copy_list);
  free(coded);
  free(intrvs);
  free(ref_cnts);

  return cali;
}

/**
 * Decodes the successor list of the node "node", stores the result
 * of the decoding in "*dest", and the number of decoded values in 
 * "*dlen". isfc must be equal to 1 in the first call.
 * intrvs can be NULL because it will be intialized anyway.
 */
void decode_CompAdjListI(CompAdjListI *cali, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s),
  uint64_t m_nb_intrs,uint64_t *intrvs) 
{
  *dlen=0;
  uint64_t u=cali->cd[node];

  if(intrvs == NULL){
    isfc=1;
    m_nb_intrs = (cali->md - 1)/cali->th+1;
    intrvs = malloc((m_nb_intrs*2+2)*sizeof(uint64_t));
  }

  if(u < cali->cd[node + 1]){
     uint64_t
      ref=decoding_function(cali->adj,&u),
      cpt=0,cptdecoded=0, 
      cptblocks=0, i=0,j, 
      block_size, nb_blocks;

    //DECODING COPY BLOCKS
    if(ref>0) {
      decode_CompAdjListI(cali, node-ref, dest, dlen, 0,
        decoding_function, m_nb_intrs, intrvs);

      uint8_t cv=1;
      nb_blocks=decoding_function(cali->adj,&u);

      while(cptblocks<nb_blocks-1){
        block_size=decoding_function(cali->adj,&u);
        if(cv){
          for(i=0;i<block_size;i++){
            dest[cptdecoded++]=dest[cpt++];
          }
        }else{
          for(i=0;i<block_size;i++)
            cpt++;
        }
        cv=!cv;
        cptblocks++;
      }
      uint64_t tmppos=*dlen-cpt;

      if(cv)
        for(i=0;i<tmppos;i++){
          dest[cptdecoded++]=dest[cpt++];
        }
    }


    //DECODING INTERVALS
    uint64_t left_ext, p_right_ext, len;

    intrvs[0] = decoding_function(cali->adj,&u);

    if(intrvs[0] > 0){
      intrvs[1] = read_nth_bit(cali->adj, u++);

      for(i=0;i<intrvs[0];i++){
        intrvs[2+i] = 
          decoding_function(cali->adj,&u);
      }
      for(i=0;i<intrvs[0];i++){
        intrvs[2+m_nb_intrs+i] = 
          decoding_function(cali->adj,&u);
      }

      if(intrvs[1])
        left_ext=node-intrvs[2];
      else
        left_ext=intrvs[2]+node;
      len=intrvs[2+m_nb_intrs]+cali->th;

      for(i=0;i<len;i++)
        dest[cptdecoded++]=left_ext+i;
      p_right_ext=dest[cptdecoded-1];

      for(i=1;i<intrvs[0];i++){
        left_ext=intrvs[2+i]+p_right_ext+2;
        len=intrvs[2+m_nb_intrs+i]+cali->th;
        for(j=0;j<len;j++)
          dest[cptdecoded++]=left_ext+j;
        p_right_ext=dest[cptdecoded-1];
      }
    }

    //DECODING THE LEFT OVER VALUES
    if(u<cali->cd[node+1]){
      dest[cptdecoded++]=inv_func(node,decoding_function(cali->adj,&u));
          
      while(u < cali->cd[node + 1]){
        dest[cptdecoded]=dest[cptdecoded-1]+decoding_function(cali->adj,&u)+1;
        cptdecoded++;
      }
    }
    *dlen=cptdecoded;
  }
  if(isfc)
    free(intrvs);
}


/**
 * Stores a CompAdjListI in a file
 */
void write_CompAdjListI(CompAdjListI *cali, uint8_t id, char *path)
{
    FILE *file=fopen(path,"wb");
    uint64_t i;
    
    fwrite(&id,sizeof(uint8_t),1,file);
    if(id==4){
        uint8_t k=getK();
        fwrite(&k,sizeof(uint8_t),1,file);
    }

    fwrite(&cali->n,sizeof(uint64_t),1,file);
    fwrite(&cali->e,sizeof(uint64_t),1,file);
    fwrite(&cali->md,sizeof(uint64_t),1,file);
   
    fwrite(&cali->nbb,sizeof(uint64_t),1,file);
    fwrite(&cali->th,sizeof(uint8_t),1,file);

    for(i=0;i<cali->n+2;i++)
      fwrite(&cali->cd[i],sizeof(uint64_t),1,file);
   
    for(i=0;i<cali->nbb;i++)
      fwrite(&cali->adj[i], sizeof(uint8_t), 1, file);

    fclose(file);
}

/**
 * Reads a CompAdjListI from a file
 */
CompAdjListI *read_CompAdjListI(char *path, uint8_t *id)
{
    FILE *file=fopen(path,"rb");
    uint64_t i;
    uint8_t k=0;
    CompAdjListI *cali=malloc(sizeof(CompAdjListI));

    assert( fread(id, sizeof(uint8_t), 1, file) == 1);
    if(*id==4){
        assert( fread(&k, sizeof(uint8_t), 1, file) == 1);
        setK(k);
    }

    assert(fread(&cali->n,sizeof(uint64_t),1,file) == 1);
    assert(fread(&cali->e,sizeof(uint64_t),1,file) == 1);
    assert(fread(&cali->md,sizeof(uint64_t),1,file) == 1);

    assert(fread(&cali->nbb,sizeof(uint64_t),1,file) == 1);
    assert(fread(&cali->th,sizeof(uint8_t),1,file) == 1);


    cali->cd=malloc((cali->n+2)*sizeof(uint64_t));
    cali->adj=malloc(cali->nbb*sizeof(uint8_t));

    for(i=0;i<cali->n+2;i++)
      assert(fread(&cali->cd[i],sizeof(uint64_t),1,file) == 1);

    for(i=0;i<cali->nbb;i++)
      assert(fread(&cali->adj[i],sizeof(uint8_t),1,file) == 1);
    
    fclose(file);
    return cali;
}

/**
 * Runs the Breadth-First Search algorithm on the CompAdjListI
 * starting from the "curr".
 * Returns the array of the visited nodes, and stores it's length
 * in "*nbvals".
 */
uint64_t *bfs_CompAdjListI(
    CompAdjListI *cali,uint64_t curr, uint64_t *nbvals,
    uint64_t decoding_function(uint8_t *arr,uint64_t *s))
{
  uint64_t m_nb_intrs=(cali->md - 1)/cali->th+1;
  uint64_t v,
    s_file=0, e_file=0,
    *file=malloc((cali->n+1)*sizeof(uint64_t)),
    *dest=malloc((cali->n+1)*sizeof(uint64_t)),
    *intrvs = malloc((m_nb_intrs*2+2)*sizeof(uint64_t)),
    dlen=0;
  uint8_t *seen=calloc(cali->n+1,sizeof(uint8_t));
  *nbvals=0;
  
  file[e_file++]=curr;
  seen[curr]=1;
  
  for(uint64_t i=s_file;i<e_file;i++){   
    v=file[i];
    seen[v]=1;
    decode_CompAdjListI(cali,v,dest,&dlen,0,
      decoding_function, m_nb_intrs, intrvs);
    
    
    for(uint64_t k=0;k<dlen;k++){
      if(!seen[dest[k]]){
        file[e_file++]=dest[k];
        seen[dest[k]]=1;
      }
    }
  }
  file=realloc(file,e_file*sizeof(uint64_t));
  *nbvals=e_file;
  free(intrvs);
  free(dest);
  free(seen);
  return file;
}