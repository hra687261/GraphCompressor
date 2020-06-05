#include "compAdjList.h"

/**
 * Frees the compressed adjlist passed as a parameter
 */
void free_CompAdjList(CompAdjList *alg)
{
  free(alg->cd);
  free(alg->adj);
  free(alg);
}

/**
 * loads a compressed adjlist from a file 
 * the file must contain a list of edges, with a pair of postive integers separated with a space 
 * the file also needs to be sorted  by the first then the second column.
 * 
 * u : writing pointer, points to the next value to be writted
 * ws : window size
 * mrc : max ref count
 */
CompAdjList *load_CompAdjList(char *path, uint8_t ign, 
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val), 
  uint64_t expl_function(uint64_t val), 
  uint64_t *u, 
  uint64_t jump, uint64_t ws, 
  uint64_t mrc, uint64_t threshold)
{
  uint64_t i, j, l = 100;
  char *line = malloc(sizeof(char) * l);
  CompAdjList *alg = malloc(sizeof(CompAdjList));
  alg->n=0;
  alg->e=0;
  alg->j=jump;
  alg->ws=ws;
  alg->th=threshold;
  uint64_t v1, v2, 
    // upper bound of the memory size we'll need to allocate (arbitrary)
    alloc_ub = 0,

    precv = 0, 
    max_deg = 0, 
    new_deg = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // counting the max degree, the number of nodes and the number of edges
  FILE *file = fopen(path, "r");
  for (i = 0; i < ign; i++)
    assert(fgets(line, l, file)); 
  while (fscanf(file, "%lu %lu", & v1, & v2) == 2) {
    if (v1 == precv) {
      new_deg++;
    } else {
      if (new_deg > max_deg)
        max_deg = new_deg;
      new_deg = 1;
      precv = v1;
    }
    alg->n = max3(alg->n, v1, v2);
    alg->e++;
    alloc_ub += expl_function(v2)*3;
  }
      if (new_deg > max_deg)
        max_deg = new_deg;
  alg->cds = (alg->n + 1)/jump + 1;
  alg->md = max_deg;
  alg->cd = calloc(alg->cds, sizeof(uint64_t));
  alg->adj = calloc(alloc_ub / 8 + 1, sizeof(uint8_t));


  uint64_t wa_len = ws + 1,
    
    // max number of intervals 
    m_nb_intrs = (max_deg - 1)/alg->th + 1;
  
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
    *window_succ = malloc(max_deg * wa_len * sizeof(uint64_t)),
    *window_pos = malloc(max_deg * wa_len * sizeof(uint64_t)),
    *window_lens = calloc(wa_len, sizeof(uint64_t)),
    *window_nodes = calloc(wa_len, sizeof(uint64_t)),
    
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
    *ref_cnts = calloc(alg->n + 1, sizeof(uint8_t));
    
    /* 
    array of 0s and 1s in which coded[x]=1 if x 
    was coded with the reference or the intervals method
    and 0 otherwise (it's used to change the positions of the nodes
    in the successors list after the compression)
    */
  uint8_t  *coded = calloc(max_deg, sizeof(uint8_t));
 
  // the number of values that were compressed with the reference technique
  uint64_t nbv_refc = 0,
  
    // the reference (c_node-ref : the referred node)
    ref = 0;

  // the last node that had it's successors list compressed
  uint64_t precun=0;

  *u=0;
  v1=0;
  uint64_t sl_len=0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // compressing the edge list
  rewind(file);
  uint8_t noprec=1;
  for (i = 0; i < ign; i++)
    assert(fgets(line, l, file));

  for(uint64_t ind=0;ind<alg->e+1;ind++){
    if(ind < alg->e)
      assert(fscanf(file, "%lu %lu", &v1, &v2) == 2);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (v1 == window_nodes[c_w_ind] 
    && ind > 0 
    && ind < alg->e) 
    {
      tmppos_2 = c_w_ind * max_deg + window_lens[c_w_ind];
      window_succ[tmppos_2] = v2;
      window_pos[tmppos_2] = window_lens[c_w_ind];
      window_lens[c_w_ind]++;
    } else {
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      it_w_ind = (c_w_ind + 1) % wa_len;
      c_node = window_nodes[c_w_ind];
      it_node = window_nodes[it_w_ind];

      nbv_refc = 0;
      ref = 0;
      /*
      if there are missing nodes, we write 0s at their bit addresses
      */
      if(c_node>precun){
        if(precun==0 && noprec==1){
          for(uint64_t i=precun; i < c_node; i++){
            encoding_function(alg->adj,u,0);
            if((i+1)%alg->j == 0)
              alg->cd[(i+1)/alg->j]=*u;
          }
        }else{
          for(uint64_t i=precun+1; i < c_node; i++){
            encoding_function(alg->adj,u,0);
            if((i+1)%alg->j == 0)
              alg->cd[(i+1)/alg->j]=*u;
          }
        }
      }

      uint64_t
        *c_arr = window_succ + max_deg * c_w_ind,
        *ca_pos = window_pos + max_deg * c_w_ind,
        ca_len = window_lens[c_w_ind];

      if(ca_len > 0){
        while (it_w_ind != c_w_ind &&
          ref_cnts[it_node] < mrc) {
          uint64_t
            *it_arr = window_succ + it_w_ind * max_deg,
            it_len = window_lens[it_w_ind];

          nb_v_tocode = 0;
          i = 0;
          j = 0;
          while (i < it_len && j < ca_len) {
            if (it_arr[i] == c_arr[j]) {
              nb_v_tocode++;
              i++;
              j++;
            } else if (it_arr[i] > c_arr[j]) {
              j++;
            } else {
              i++;
            }
          }
          if (nb_v_tocode > nbv_refc) {
            nbv_refc = nb_v_tocode;
            ref = c_node - it_node;
            ref_w_ind = it_w_ind;
          }
          it_w_ind = (it_w_ind + 1) % wa_len;
          it_node = window_nodes[it_w_ind];
        }


        uint64_t ra_len=0,nb_blocks=0;
        uint8_t cv;
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
            *ra_pos = window_pos + max_deg * ref_w_ind;
            
            // the length of the referenced list 
            ra_len = window_lens[ref_w_ind];

          // building the copy_list
          while (ra_it < ra_len && ca_it < ca_len) {
            if (r_arr[ra_it] == c_arr[ca_it]) {
              ca_pos[ca_it] = p1++;
              coded[ca_pos[ca_it++]] = 1;
              copy_list[ra_pos[ra_it++]] = 1;
            } else if (r_arr[ra_it] > c_arr[ca_it]) {
              ca_pos[ca_it] = p2++;
              ca_it++;
            } else {
              ra_it++;
            }
          }

          // updating the positions of the nodes in the current array (the one we're about to compress)
          uint64_t cpt=0;
          for(i=0;i<ra_len;i++){
            if(copy_list[i]){
              cpt+=copy_list[i];
              copy_list[i]=cpt;
            }
          }

          ra_it=0;
          ca_it=0;
          while (ra_it < ra_len && ca_it < ca_len){
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
          }
          // the first block is always a 1 block
          cv=1;
          // the number copy blocks
          nb_blocks=1;

          //counting the number of blocks
          for (i = 0; i < ra_len; i++) 
            if(copy_list[i] != cv){
              nb_blocks++;
              cv=!cv;
            }
        }
  
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        uint64_t 
          c_rightext = 0, 
          c_leftext = 0, v,
          prec_rightext = c_node,
          cpt = 1,
          nbv_intrvc = 0;

        precv=window_succ[max_deg*c_w_ind];
        i=1;

        // building the intervals
        intrvs[1]=0;
        c_leftext=c_arr[0];
        cpt=1;

        uint8_t isf=1;
        do{
          v=c_arr[i];
          if( precv + 1 == v 
            && coded[ca_pos[i]]==0 
            && coded[ca_pos[i-1]]==0) 
          { 
            cpt++;
            c_rightext=v;
            if(i==ca_len-1){
              if(cpt>=alg->th){
                if(isf){
                if(c_leftext >= prec_rightext){
                  intrvs[1]=0;
                  intrvs[2+intrvs[0]]=c_leftext-prec_rightext;
                }else{
                  intrvs[1]=1;
                  intrvs[2+intrvs[0]]=prec_rightext-c_leftext;
                }
                isf=0;
                }else{
                  intrvs[2+intrvs[0]]=c_leftext-prec_rightext;
                }
                intrvs[2+m_nb_intrs+intrvs[0]]=cpt-alg->th;
                intrvs[0]++;
                nbv_intrvc+=cpt;
                for(j=i-cpt+1;j<=i;j++){
                  coded[ca_pos[j]]=2;
                }
              }
            }
          } else {
            if(cpt>=alg->th){
              if(isf){
                if(c_leftext >= prec_rightext){
                  intrvs[1]=0;
                  intrvs[2+intrvs[0]]=c_leftext-prec_rightext;
                }else{
                  intrvs[1]=1;
                  intrvs[2+intrvs[0]]=prec_rightext-c_leftext;
                }
                isf=0;
              }else{
                intrvs[2+intrvs[0]]=c_leftext-prec_rightext;
              }
              intrvs[2+m_nb_intrs+intrvs[0]]=cpt-alg->th;
              intrvs[0]++;
              nbv_intrvc+=cpt;
              for(j=i-cpt;j<i;j++)
                coded[ca_pos[j]]=2;
              prec_rightext=c_rightext+2;
            }
            c_leftext=v;
            cpt=1;
          }

          precv=v;
          i++;
        }while( i < ca_len );


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Calculating the number of bits needed to encode the list of successors
        uint64_t
          p1=nbv_refc,
          p2=nbv_refc+nbv_intrvc;

        for(i=0;i<ca_len;i++){
          if(coded[ca_pos[i]]==2){
            ca_pos[i]=p1++;
          } else if(coded[ca_pos[i]]==0){
            ca_pos[i]=p2++;
          } 
        }

        sl_len+=expl_function(ref);
        uint64_t cpt0=0, cpt1=0; 
        cpt=0;
        cv=1;
        if(ref>0){
          sl_len+=expl_function(nb_blocks);
          for (i = 0; i < ra_len; i++) {
            if(cv){
              if(copy_list[i])
                cpt1++;
              else{
                sl_len+=expl_function(cpt1);
                cpt++;
                cpt0=1;
                cv=!cv;
              }
            }else{
              if(copy_list[i]){
                sl_len+=expl_function(cpt0);
                cpt++;
                cpt1=1;
                cv=!cv;
              }else
                cpt0++;
            }
            if(cpt==nb_blocks-1)
              break;
          }
        }
        sl_len+=expl_function(intrvs[0]);

        if(intrvs[0] > 0){
          sl_len+=1;
          for(i=0;i<intrvs[0];i++){
            sl_len+=expl_function(intrvs[2+i]);
            sl_len+=expl_function(intrvs[2+m_nb_intrs+i]);
          }
        }

        for(i=0;i<ca_len;i++){
          if(ca_pos[i] > nbv_refc+nbv_intrvc){
            sl_len+=expl_function(c_arr[i] - precv - 1);
            precv = c_arr[i];
            continue;
          }
          if(ca_pos[i] == nbv_refc+nbv_intrvc){
            precv = c_arr[i];
            sl_len+=expl_function(func(c_node, precv));
          }
        }  

        //////////////////////////////////////////////////////////////////////////

        //writing the size of the offset array
        encoding_function(alg->adj, u, sl_len);
        
        //writing the reference
        encoding_function(alg->adj, u, ref);
        
        //Writing CopyBlocks
        if(ref>0)
        {
          //writing the number of copy blocks
          encoding_function(alg->adj, u, nb_blocks);
          
          cpt0=0; cpt1=0; cpt=0;
          cv=1;

          ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          //writing the copy blocks
          for (i = 0; i < ra_len && cpt < nb_blocks-1; i++) {
            if(cv){
              if(copy_list[i])
                cpt1++;
              else{
                encoding_function(alg->adj, u, cpt1);
                cpt++;
                cpt0=1;
                cv=!cv;
              }
            }else{
              if(copy_list[i]){
                encoding_function(alg->adj, u, cpt0);
                cpt++;
                cpt1=1;
                cv=!cv;
              }else
                cpt0++;
            }
            copy_list[i]=0;
          }
          while(i<ra_len){
            copy_list[i]=0;
            i++;
          }
        }

        //Writing intervals
        {   
          //writing the number of intervals
          encoding_function(alg->adj,u,intrvs[0]);
        
          if(intrvs[0] > 0){
            //writing the the left extremes
            if(intrvs[1]){
              set_nth_bit(alg->adj,u,*u);
            }else{
              clear_nth_bit(alg->adj,u,*u);
            }

            for(i=0;i<intrvs[0];i++){
              encoding_function(alg->adj,u,intrvs[2+i]);
            }
            
            //writing the the left extremes
            for(i=0;i<intrvs[0];i++){
              encoding_function(alg->adj,u,intrvs[2+m_nb_intrs+i]);
            }  
          }
        }

        // writing the leftover values, the ones that weren't compressed
        for(i=0;i<ca_len;i++){
          coded[i]=0;
          if(ca_pos[i] > nbv_refc+nbv_intrvc){
            encoding_function(alg->adj, u, 
              c_arr[i] - precv - 1);
            precv = c_arr[i];
            continue;
          }

          //the first left over node
          if(ca_pos[i] == nbv_refc+nbv_intrvc){
            precv = c_arr[i];
            encoding_function(alg->adj, u, 
              func(c_node, precv));
          }
        }  
      }   
        
      if((c_node+1)%alg->j == 0){
        alg->cd[(c_node+1)/alg->j]=*u;
      }

      precun=c_node;
      if(noprec) noprec=0;
      c_w_ind = (c_w_ind + 1) % wa_len;
      window_nodes[c_w_ind] = v1;

      tmppos_2 = c_w_ind * max_deg;
      window_succ[tmppos_2] = v2;
      window_pos[tmppos_2] = 0;
      window_lens[c_w_ind] = 1;
      intrvs[0]=0;
      sl_len=0;
    }
  }

  if(alg->n > precun){
    for(i=precun;i<=alg->n;i++){
      encoding_function(alg->adj, u, 0);
      if((i+1)%alg->j == 0)
        alg->cd[(i+1)/alg->j]=*u;
    }
  }

  alg->nbb = ((*u - 1) / 8 + 1);
  alg->adj = realloc(alg->adj,alg->nbb * sizeof(uint8_t));
  
  fclose(file);
  free(line);
  free(window_succ);
  free(window_pos);
  free(window_lens);
  free(window_nodes);
  free(copy_list);
  free(coded);
  free(intrvs);
  free(ref_cnts);

  return alg;
}

/**
 * returns the position of the node's successor list in the graph
 */
uint64_t get_node_addrs(
  uint64_t node, CompAdjList *alg,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s))
{
  uint64_t pos, l, u=alg->cd[node/alg->j];
  pos=u;
  for(uint64_t i=0; i < (node%alg->j); i++){
    l=decoding_function(alg->adj,&u);
    pos=u+l;
    u=pos;
  }
  return pos;
}


/**
 * Stores a compressed adjlist in a file
 */
void write_CompAdjList(CompAdjList *alg, uint8_t id, char *path)
{
    FILE *file=fopen(path,"wb");
    uint64_t i;
    
    fwrite(&id,sizeof(uint8_t),1,file);
    if(id==4){
        uint8_t k=getK();
        fwrite(&k,sizeof(uint8_t),1,file);
    }

    fwrite(&alg->n,sizeof(uint64_t),1,file);
    fwrite(&alg->e,sizeof(uint64_t),1,file);
    fwrite(&(alg->md),sizeof(uint64_t),1,file);

    fwrite(&alg->nbb,sizeof(uint64_t),1,file);
    fwrite(&alg->cds,sizeof(uint64_t),1,file);

    fwrite(&(alg->ws),sizeof(uint64_t),1,file);
    fwrite(&(alg->j),sizeof(uint64_t),1,file);
    fwrite(&(alg->th),sizeof(uint64_t),1,file);
    
    for(i=0;i<alg->cds;i++)
        fwrite(&alg->cd[i],sizeof(uint64_t),1,file);
   
    for(i=0;i<alg->nbb;i++)
        fwrite(&alg->adj[i], sizeof(uint8_t), 1, file);

    fclose(file);
}

/**
 * Reads a compressed adjlist from a file
 */
CompAdjList *read_CompAdjList(char *path, uint8_t *id)
{
    FILE *file=fopen(path,"rb");
    uint64_t i;
    uint8_t k=0;
    CompAdjList *alg=malloc(sizeof(CompAdjList));

    assert( fread(id, sizeof(uint8_t), 1, file) == 1);
    if(*id==4){
        assert( fread(&k, sizeof(uint8_t), 1, file) == 1);
        setK(k);
    }

    assert(fread(&(alg->n),sizeof(uint64_t),1,file) == 1);
    assert(fread(&(alg->e),sizeof(uint64_t),1,file) == 1);
    assert(fread(&(alg->md),sizeof(uint64_t),1,file) == 1);

    assert(fread(&(alg->nbb),sizeof(uint64_t),1,file) == 1);
    assert(fread(&(alg->cds),sizeof(uint64_t),1,file) == 1);
    
    assert(fread(&(alg->ws),sizeof(uint64_t),1,file) == 1);
    assert(fread(&(alg->j),sizeof(uint64_t),1,file) == 1);
    assert(fread(&(alg->th),sizeof(uint64_t),1,file) == 1);

    alg->cd=malloc((alg->cds)*sizeof(uint64_t));
    alg->adj=malloc(alg->nbb*sizeof(uint8_t));

    for(i=0;i<alg->cds;i++)
        assert(fread(&alg->cd[i], sizeof(uint64_t), 1, file) == 1);

    for(i=0;i<alg->nbb;i++)
        assert( fread(&(alg->adj[i]), sizeof(uint8_t), 1, file) == 1);
    
    fclose(file);
    return alg;
}



void decode_CompAdjList(CompAdjList *alg, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s),
  uint64_t m_nb_intrs,uint64_t *intrvs)
{
  *dlen=0;
  uint64_t u=get_node_addrs(node,alg,decoding_function);
  if(intrvs == NULL){
    isfc=1;
    m_nb_intrs = (alg->md - 1)/alg->th+1;
    intrvs = malloc((m_nb_intrs*2+2)*sizeof(uint64_t));
  }
  uint64_t clen;
  clen=decoding_function(alg->adj,&u);
  clen+=u;
  if(u < clen){
     uint64_t
      ref=decoding_function(alg->adj,&u),
      cpt=0,cptdecoded=0, 
      cptblocks=0, i=0,j, 
      block_size, nb_blocks;

    //DECODING COPY BLOCKS
    if(ref>0) {
      decode_CompAdjList(alg, node-ref, dest, dlen, 0,
        decoding_function, m_nb_intrs, intrvs);
      
      uint8_t cv=1;
      nb_blocks=decoding_function(alg->adj,&u);

      while(cptblocks<nb_blocks-1){
        block_size=decoding_function(alg->adj,&u);
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
    intrvs[0] = decoding_function(alg->adj,&u);
    if(intrvs[0] > 0){
      intrvs[1] = read_nth_bit(alg->adj, u++);
      for(i=0;i<intrvs[0];i++){
        intrvs[2+i] = 
          decoding_function(alg->adj,&u);
      }
      for(i=0;i<intrvs[0];i++){
        intrvs[2+m_nb_intrs+i] = 
          decoding_function(alg->adj,&u);
      }

      if(intrvs[1])
        left_ext=node-intrvs[2];
      else
        left_ext=intrvs[2]+node;
      len=intrvs[2+m_nb_intrs]+alg->th;
      for(i=0;i<len;i++)
        dest[cptdecoded++]=left_ext+i;
      p_right_ext=dest[cptdecoded-1];

      for(i=1;i<intrvs[0];i++){
        left_ext=intrvs[2+i]+p_right_ext+2;
        len=intrvs[2+m_nb_intrs+i]+alg->th;
        for(j=0;j<len;j++)
          dest[cptdecoded++]=left_ext+j;
        p_right_ext=dest[cptdecoded-1]; 
      }
    }
    
    //DECODING THE LEFT OVER VALUES
    if(u<clen){
      dest[cptdecoded++]=inv_func(node,decoding_function(alg->adj,&u));
      while(u < clen){
        dest[cptdecoded]=dest[cptdecoded-1]+decoding_function(alg->adj,&u)+1;
        cptdecoded++;
      }
    }
    *dlen=cptdecoded;
  }
  if(isfc)
    free(intrvs);
}


uint64_t *bfs_CompAdjList(
  CompAdjList *alg,uint64_t curr, uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s))
{
  uint64_t m_nb_intrs=(alg->md - 1)/alg->th+1;
  uint64_t v,
    s_file=0, e_file=0,
    *file=malloc((alg->n+1)*sizeof(uint64_t)),
    *dest=malloc((alg->n+1)*sizeof(uint64_t)),
    *intrvs = malloc((m_nb_intrs*2+2)*sizeof(uint64_t)),
    dlen=0;
  uint8_t *seen=calloc(alg->n+1,sizeof(uint8_t));
  *nbvals=0;
  
  file[e_file++]=curr;
  seen[curr]=1;
  
  for(uint64_t i=s_file;i<e_file;i++){   
    v=file[i];
    seen[v]=1;
    decode_CompAdjList(alg,v,dest,&dlen, 0,
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


void decode_CompAdjList_bis(CompAdjList *alg, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s),
  uint64_t m_nb_intrs,uint64_t *intrvs,
  uint64_t **hist, uint64_t *lens)
{

  if(hist[node] != NULL) {
    *dlen=lens[node];
    for(uint64_t i=0;i<*dlen;i++)
      dest[i]=hist[node][i];
  }else{
  *dlen=0;
  uint64_t u=get_node_addrs(node,alg,decoding_function);

  if(intrvs == NULL){
    isfc=1;
    m_nb_intrs = (alg->md - 1)/alg->th+1;
    intrvs = malloc((m_nb_intrs*2+2)*sizeof(uint64_t));
  }
  uint64_t clen;
  clen=decoding_function(alg->adj,&u);
  clen+=u;
  if(u < clen){
     uint64_t
      ref=decoding_function(alg->adj,&u),
      cpt=0,cptdecoded=0, 
      cptblocks=0, i=0,j, 
      block_size, nb_blocks;

    //DECODING COPY BLOCKS
    if(ref>0) {
      decode_CompAdjList_bis(alg, node-ref, dest, dlen, 0,
        decoding_function, m_nb_intrs, intrvs, hist, lens);
      uint8_t cv=1;
      nb_blocks=decoding_function(alg->adj,&u);
      while(cptblocks<nb_blocks-1){
        block_size=decoding_function(alg->adj,&u);
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
    intrvs[0] = decoding_function(alg->adj,&u);
    if(intrvs[0] > 0){
      intrvs[1] = read_nth_bit(alg->adj, u++);

      for(i=0;i<intrvs[0];i++){
        intrvs[2+i] = 
          decoding_function(alg->adj,&u);
      }
      for(i=0;i<intrvs[0];i++){
        intrvs[2+m_nb_intrs+i] = 
          decoding_function(alg->adj,&u);
      }
      if(intrvs[1])
        left_ext=node-intrvs[2];
      else
        left_ext=intrvs[2]+node;
      len=intrvs[2+m_nb_intrs]+alg->th;
      for(i=0;i<len;i++)
        dest[cptdecoded++]=left_ext+i;
      
      p_right_ext=dest[cptdecoded-1];

      for(i=1;i<intrvs[0];i++){
        left_ext=intrvs[2+i]+p_right_ext+2;
        len=intrvs[2+m_nb_intrs+i]+alg->th;
       
        for(j=0;j<len;j++)
          dest[cptdecoded++]=left_ext+j;
        p_right_ext=dest[cptdecoded-1]; 
      }
    }
      //DECODING THE LEFT OVER VALUES
    if(u<clen){
      dest[cptdecoded++]=inv_func(node,decoding_function(alg->adj,&u));
      while(u < clen){
        dest[cptdecoded]=dest[cptdecoded-1]+decoding_function(alg->adj,&u)+1;
        cptdecoded++;
      }
    }
    *dlen=cptdecoded;
  }

  hist[node] = malloc((*dlen)*sizeof(uint64_t));
  lens[node] = *dlen;
  for(uint64_t i=0;i<(*dlen);i++){
    hist[node][i]=dest[i];
  }
  if(isfc)
    free(intrvs);
  
  }
}

uint64_t *bfs_CompAdjList_bis(
  CompAdjList *alg,uint64_t curr, uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s))
{
  uint64_t m_nb_intrs=(alg->md - 1)/alg->th+1;
  uint64_t v, i,
    s_file=0, e_file=0,
    *file=malloc((alg->n+1)*sizeof(uint64_t)),
    *dest=malloc((alg->n+1)*sizeof(uint64_t)),
    *intrvs = malloc((m_nb_intrs*2+2)*sizeof(uint64_t)),
    dlen=0;
  uint8_t *seen=calloc(alg->n+1,sizeof(uint8_t));
  *nbvals=0;

  uint64_t **hist=malloc((alg->n+1)*sizeof(uint64_t *)),
            *lens=malloc((alg->n+1)*sizeof(uint64_t));
  
  for(i=0;i<alg->n+1;i++)
    hist[i]=NULL;

  file[e_file++]=curr;
  seen[curr]=1;
  
  for(i=s_file;i<e_file;i++){   
    v=file[i];
    seen[v]=1;
    decode_CompAdjList_bis(alg,v,dest,&dlen, 0,
      decoding_function, m_nb_intrs, intrvs,hist,lens);

    for(uint64_t k=0;k<dlen;k++){
      if(!seen[dest[k]]){
        file[e_file++]=dest[k];
        seen[dest[k]]=1;
      }
    }
  }
  file=realloc(file,e_file*sizeof(uint64_t));
  *nbvals=e_file;
  for(i=0;i<alg->n+1;i++)
    if(hist[i]==NULL)
      free(hist[i]);
  free(lens);
  free(hist);
  free(intrvs);
  free(dest);
  free(seen);
  return file;
}


void decode_with_window(
  CompAdjList *alg, uint64_t node,
  uint64_t *window_succ,
  uint64_t *window_nodes,
  uint64_t *window_lens,
  uint64_t *ldecp, 
  uint64_t decoding_function(uint8_t *arr, uint64_t *s))
{

  uint64_t e_bit,
    u=get_node_addrs(node,alg,decoding_function),
    m_nb_intrs = (alg->md - 1)/alg->th+1;
  
  e_bit=decoding_function(alg->adj,&u);
  e_bit+=u;


  if(u < e_bit){
    uint64_t *intrvs = malloc((m_nb_intrs*2+2)*sizeof(uint64_t));

    uint64_t c_it=((*ldecp)+1)%(alg->ws+1), *c_arr;
    (*ldecp)=((*ldecp)+1)%(alg->ws+1);
    c_arr = window_succ+alg->md*c_it;
    window_lens[c_it]=0;
    window_nodes[c_it]=node;

    uint64_t 
      r_node, 
      *r_arr, r_len, 
      r_it=(c_it+1)%(alg->ws+1);

    uint64_t
      ref=decoding_function(alg->adj,&u),
      cpt=0, 
      cptblocks=0, i=0,j, 
      block_size, nb_blocks;
    r_node=node-ref;

    if(ref>0) {
      
      while(window_nodes[r_it]!=r_node || window_lens[r_it]==0){
        r_it=(r_it+1)%(alg->ws+1);
      }

      r_arr = window_succ+alg->md*r_it;  
      r_len = window_lens[r_it];

      uint8_t cv=1;
      nb_blocks=decoding_function(alg->adj,&u);

      while(cptblocks<nb_blocks-1){
        block_size=decoding_function(alg->adj,&u);
        if(cv){
          for(i=0;i<block_size;i++){
            c_arr[window_lens[c_it]++]=r_arr[cpt++];
          }
        }else{
          for(i=0;i<block_size;i++)
            cpt++;
        }
        cv=!cv;
        cptblocks++;
      }

      uint64_t tmppos=r_len-cpt;
      if(cv)
        for(i=0;i<tmppos;i++){
          c_arr[window_lens[c_it]++]=r_arr[cpt++];
        }
    }
    
    //DECODING INTERVALS
    uint64_t left_ext, p_right_ext, len;
    intrvs[0] = decoding_function(alg->adj,&u);

    if(intrvs[0] > 0){
      intrvs[1] = read_nth_bit(alg->adj, u++);
      for(i=0;i<intrvs[0];i++){
        intrvs[2+i] = 
          decoding_function(alg->adj,&u);
      }
      for(i=0;i<intrvs[0];i++){
        intrvs[2+m_nb_intrs+i] = 
          decoding_function(alg->adj,&u);
      }

      if(intrvs[1])
        left_ext=node-intrvs[2];
      else
        left_ext=intrvs[2]+node;
        
      len=intrvs[2+m_nb_intrs]+alg->th;
      for(i=0;i<len;i++)
        c_arr[window_lens[c_it]++]=left_ext+i;
      p_right_ext=c_arr[window_lens[c_it]-1];

      for(i=1;i<intrvs[0];i++){
        left_ext=intrvs[2+i]+p_right_ext+2;
        len=intrvs[2+m_nb_intrs+i]+alg->th;
        for(j=0;j<len;j++)
          c_arr[window_lens[c_it]++]=left_ext+j;
        p_right_ext=c_arr[window_lens[c_it]-1]; 
      }
    }

    //DECODING THE LEFT OVER VALUES
    if(u<e_bit){
      c_arr[window_lens[c_it]++]=
        inv_func(node,decoding_function(alg->adj,&u));
      while(u < e_bit){
        c_arr[window_lens[c_it]]=c_arr[window_lens[c_it]-1]
          +decoding_function(alg->adj,&u)+1;
        window_lens[c_it]++;
      }
    }  
    free(intrvs);
  }
}





/**
 * normalizes the pagerank of every node
 * by deviding it by the 1-norm of the pagerank vector
 */
void normalize (CompAdjList *al, uint8_t *has_successors, double *pr)
{
	uint64_t i;
	double one_norm = 0;
  
  for (i = 0; i < al->n+1; i++)
    if(has_successors[i])
      one_norm += pr[i];
    
  for (i = 0; i < al->n+1; i++)
        if(has_successors[i])
    	    pr[i] /= one_norm;
}


/**
 * Makes and returns an array of 1 and 0, where a cell i is equal to 1
 * if the node i in the compressed graph al has successors and 0 if it doesn't
 */
uint8_t *mk_has_successors(
  CompAdjList *al,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s))
{
  uint64_t u=0,cpt=0,slen;
  uint8_t *has_succ=malloc((al->n+1)*sizeof(uint8_t)); 
  while(cpt<=al->n){
    slen=decoding_function(al->adj,&u);
    has_succ[cpt++]=slen>0;
    u+=slen;
  }
  return has_succ;
}

/**
 * Does nb it power iterations, over the links of the compressed graph al,
 * stores the result in the pagerank vector pr
 * has_succ is the array return by the function mk_has_successors
 */

double *power_iteration(
  CompAdjList *al, uint64_t nb_it, double alpha,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s))
{
  uint64_t i, in, j, ldecp=0;
  double *new_pr=malloc((al->n+1)*sizeof(double)),
         *pr=malloc((al->n+1)*sizeof(double));
  uint8_t *has_succ=mk_has_successors(al,decoding_function);
  
  
  uint64_t 
    *window_succ=malloc((al->md*(al->ws+1))*sizeof(uint64_t)),
    *window_nodes=malloc((al->ws+1)*sizeof(uint64_t)),
    *window_lens=calloc((al->n+1),sizeof(uint64_t));


  for(i=0;i<al->n+1;i++){
    pr[i]=0.0;
    new_pr[i]=0.0;
  }

  for(i=0;i<nb_it;i++){
    //iterate over all the nodes
    for(in=0;in<al->n+1;in++){
      if(has_succ[i]){
        decode_with_window( al, i, window_succ,
          window_nodes, window_lens,
          &ldecp, decoding_function);
        for(j=0;j<window_lens[ldecp];j++){
          new_pr[window_succ[ldecp*al->md+j]]+=pr[i]*(1/window_lens[ldecp]);
        }
      }
    }
    for(in=0;in<al->n+1;in++){
      if(has_succ[i])
        pr[in]=(1.-alpha)*new_pr[in]+alpha*(1./(al->n+1));
      new_pr[in]=0;
    }
    normalize(al,has_succ,pr);
  }
  free(new_pr);
  free(has_succ);
  free(window_lens);
  free(window_nodes);
  free(window_succ);
  
  return pr;
}






