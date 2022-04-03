#include "compAdjListCB.h"

/**
 * Frees a CompAdjListCB and it's content.
 */
void free_CompAdjListCB(CompAdjListCB *calcb)
{
  free(calcb->ccscb);
  free(calcb->adj);
  free(calcb->cd);
  free(calcb);
}

/**
 * Loads a CompAdjListCB from a file containing an edgelist,
 * "path" is the path to the file, the file must contain on every line 
 * a pair of positive numbers x y, separated by a space, in which x->y is a link,
 * the list needs to be sorted in increasing order by the first, then the 
 * second column.
 * 
 * "*u" points to first bit after the last used bit for the array of 
 * compressed successor lists.
 * ws: the window size.
 * mrc: max ref count.
 */
CompAdjListCB *load_CompAdjListCB(char *path,
  CompCodes_CB *ccscb , uint64_t *u, 
  uint8_t ws, uint8_t mrc)
{
  uint64_t i, j;
  CompAdjListCB *calcb = malloc(sizeof(CompAdjListCB));
  calcb->n = 0;
  calcb->e = 0;
  calcb->ccscb = ccscb;

  uint64_t v1, v2, 
    // upper bound of the memory size we'll need to allocate (arbitrary)
    alloc_ub = 0,
    precv = 0, 
    max_deg = 0, 
    new_deg = 0;

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
    calcb->n = max3(calcb->n, v1, v2);
    calcb->e++;
  }
  if (new_deg > max_deg)
    max_deg = new_deg;

  if(calcb->e == 0) {
    fclose(file);
    return calcb;
  }   

  alloc_ub = ((calcb->e + 1) * UB_BITS_PER_LINK_I) / 8 + 1;
  calcb->cd = calloc(calcb->n + 2, sizeof(uint64_t));
  calcb->adj = calloc(alloc_ub, sizeof(uint8_t));

  uint64_t 
    wa_len = ws + 1;
  
  uint64_t 
    //c_w_ind = 0, it_w_ind = 0, ref_w_ind,
    //  We represent the window with 4 arrays
    //    window_succ: contains the consecutive lists of successors that are part of the window plus
    //      the list of successors of the node we are going to compress.
    //    window_pos: contains for each source node of the lists of successors in window_succ it's position 
    //      after the compressiong the list. Because after compressing the the list of successors 
    //      the order of the nodes might be change.
    //    window_lens: contains the lengths of the lists (of successors and their positions) that are part 
    //      of the window.
    //    window_nodes : contains the nodes that are part of the window.
    *window_succ = malloc(max_deg * wa_len * sizeof(uint64_t)),
    *window_pos = malloc(max_deg * wa_len * sizeof(uint64_t)),
    *window_lens = calloc(wa_len, sizeof(uint64_t)),
    *window_nodes = calloc(wa_len, sizeof(uint64_t)),
    
    c_node = 0, it_node, 
    tmppos_2, nb_v_tocode,

    // copy list the list of copied nodes from the referenced list of successors
    *copy_list = calloc(max_deg, sizeof(uint64_t));

  //  contains the legnth of the reference chain of each list of successors
  //    ex : 
  //    if 15 references 12 which references 5 witch doesn't refence any other node then :
  //    ref_cnts[5]=0, ref_cnts[12]=1, ref_cnts[15]=2 
  uint8_t *ref_cnts = calloc(calcb->n + 1, sizeof(uint8_t)),
    
    //  array of 0s and 1s in which coded[x]=1 if x 
    //  was coded with the reference or the intervals method
    //  and 0 otherwise (it's used to change the positions of the nodes
    //  in the successors list after the compression)
    *coded = calloc(max_deg, sizeof(uint8_t));

  // the last node that had it's successors list compressed
  uint64_t precun = 0;

  *u = 0;
  v1 = 0;

  // making an array of expl functions and an array of encoding functions
  // every function is placed in the cell corresponding to it's id 
  void (**encoding_functions)(uint8_t *arr, uint64_t *u, uint64_t val, ...) 
    = calloc(NB_CODES,sizeof(&unary_encoding));

  for(i = 0; i < NB_SECTIONS_CB; i++){
    uint8_t tmp_ind = ccscb->ccscb_array[i * 2];
    if(encoding_functions[tmp_ind] == NULL) 
      encoding_functions[tmp_ind] = get_encoding_function(tmp_ind);
  }


  // compressing the edge list
  rewind(file);

  // reading the first link    
  uint64_t c_w_ind = 0, it_w_ind = 0, ref_w_ind;
  assert(fscanf(file, "%lu %lu", &v1, &v2) == 2);
  
  window_nodes[0] = v1;
  window_pos[0] = 0;
  window_lens[0] = 1;
  window_succ[0] = v2;

  // if v1 > 0, that means that the source node of the first link ins't the destination node of any link
  // and that the the nodes having a value smaller than v1, don't have any successors.
  // therefore the sizes of their encoded lists of successors are set to 0.
  if (v1 > 0)
    precun = v1 - 1;
  
  // reading the other links 
  for (uint64_t ind = 1; ind < calcb->e + 1; ind++) {
    if (ind < calcb->e){
      assert(fscanf(file, "%lu %lu", &v1, &v2) == 2);
      if (v1 == window_nodes[c_w_ind]){
        tmppos_2 = c_w_ind * max_deg + window_lens[c_w_ind];
        window_succ[tmppos_2] = v2;
        window_pos[tmppos_2] = window_lens[c_w_ind];
        window_lens[c_w_ind]++; 
        continue;
      }
    }

    it_w_ind = (c_w_ind + 1) % wa_len;
    c_node = window_nodes[c_w_ind];
    it_node = window_nodes[it_w_ind];
    
    // the number of values that were compressed with the reference technique
    uint64_t nbv_refc = 0,
      // the reference (c_node-ref : the referred node)
      ref = 0;

    for (i = precun + 1; i <= c_node; i++)
      calcb->cd[i] = *u;
    
    uint64_t
      *c_arr = window_succ + max_deg * c_w_ind,
      *ca_pos = window_pos + max_deg * c_w_ind,
      ca_len = window_lens[c_w_ind];

    if (ca_len > 0) {
      // finding a list to reference
      while (it_w_ind != c_w_ind &&
        ref_cnts[it_node] < mrc) 
      {
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
      
      uint64_t ra_len = 0, nb_blocks = 0;
      uint8_t cv;
      // building the copy blocks 
      if (ref > 0) {
        // updating the ref count
        ref_cnts[c_node] = ref_cnts[window_nodes[ref_w_ind]] + 1;

        uint64_t p1 = 0, p2 = nbv_refc,
          ra_it = 0,
          ca_it = 0,
          // the referenced list of successors (sorted in incremental order)
          *r_arr = window_succ + max_deg * ref_w_ind,
          // the real positions of the nodes in the referenced list of successors after compression
          // (if the list was compressed)
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
        uint64_t cpt = 0;
        for (i = 0; i < ra_len; i++) {
          if (copy_list[i]) {
            cpt += copy_list[i];
            copy_list[i] = cpt;
          }
        }

        ra_it = 0;
        ca_it = 0;
        while (ra_it < ra_len && ca_it < ca_len) {
          if (r_arr[ra_it] == c_arr[ca_it]) {
            ca_pos[ca_it] = copy_list[ra_pos[ra_it]] - 1;
            copy_list[ra_pos[ra_it]] = 1;
            ca_it++;
            ra_it++;
          } else if (r_arr[ra_it] > c_arr[ca_it]) {
            ca_it++;
          } else {
            ra_it++;
          }
        }
        // the first block is always a 1 block
        cv = 1;
        // the number copy blocks
        nb_blocks = 1;

        // counting the number of blocks
        for (i = 0; i < ra_len; i++)
          if (copy_list[i] != cv) {
            nb_blocks++;
            cv = !cv;
          }
      } 
        
      uint64_t
        cpt = 1,
        nbv_intrvc = 0;

      precv = window_succ[max_deg * c_w_ind];

      // writing the reference
      encoding_functions[ccscb->ccscb_struct.f_ref](calcb->adj, u, ref, ccscb->ccscb_struct.ref_k);
      uint64_t cpt0 = 0, cpt1 = 0;
      // Writing CopyBlocks
      if (ref > 0) {
        // writing the number of copy blocks
        encoding_functions[ccscb->ccscb_struct.f_bcnt](calcb->adj, u, nb_blocks, ccscb->ccscb_struct.bcnt_k);
        
        cpt0 = 0;
        cpt1 = 0;
        cpt = 0;
        cv = 1;

        // writing the copy blocks
        for (i = 0; i < ra_len && cpt < nb_blocks - 1; i++) {
          if (cv) {
            if (copy_list[i])
              cpt1++;
            else {
              encoding_functions[ccscb->ccscb_struct.f_cb](calcb->adj, u, cpt1, ccscb->ccscb_struct.cb_k);
              cpt++;
              cpt0 = 1;
              cv = !cv;
            }
          } else {
            if (copy_list[i]) {
              encoding_functions[ccscb->ccscb_struct.f_cb](calcb->adj, u, cpt0, ccscb->ccscb_struct.cb_k);
              cpt++;
              cpt1 = 1;
              cv = !cv;
            } else
              cpt0++;
          }
          copy_list[i] = 0;
        }
        
        while (i < ra_len) {
          copy_list[i] = 0;
          i++;
        }
      }

      // writing the residual values, the ones that weren't compressed
      for (i = 0; i < ca_len; i++) {
        coded[i] = 0;
        if (ca_pos[i] > nbv_refc + nbv_intrvc) {
          encoding_functions[ccscb->ccscb_struct.f_res](calcb->adj, u, c_arr[i] - precv - 1, ccscb->ccscb_struct.res_k);
          precv = c_arr[i];
          continue;
        }
        // writing the first residual node
        if (ca_pos[i] == nbv_refc + nbv_intrvc) {
          precv = c_arr[i];
          encoding_functions[ccscb->ccscb_struct.f_res](calcb->adj, u, v_func(c_node, precv), ccscb->ccscb_struct.res_k);
        }
      }
    }
    precun = c_node;
    c_w_ind = (c_w_ind + 1) % wa_len;
    window_nodes[c_w_ind] = v1;
    
    tmppos_2 = c_w_ind * max_deg;
    window_succ[tmppos_2] = v2;
    window_pos[tmppos_2] = 0;
    window_lens[c_w_ind] = 1;
  }
  for(i = precun + 1; i <= calcb->n + 1; i++)
    calcb->cd[i] = *u;

  calcb->nbb = (( *u - 1) / 8 + 1);
  calcb->adj = realloc(calcb->adj, calcb->nbb * sizeof(uint8_t));

  fclose(file);
  free(encoding_functions);
  free(window_succ);
  free(window_pos);
  free(window_lens);
  free(window_nodes);
  free(copy_list);
  free(coded);
  free(ref_cnts);

  return calcb;
}

/**
 * Stores a CompAdjListCB in a file.
 */
void write_CompAdjListCB(CompAdjListCB *calcb, char *path)
{
  FILE *file=fopen(path,"wb");
  uint64_t i;

  for(i = 0; i < NB_SECTIONS_CB; i++){
    uint64_t tmp = calcb->ccscb->ccscb_array[i * 2];
    fwrite(&tmp, sizeof(uint8_t), 1, file);
    if(tmp == ZETA_K_FUNC_ID)
      fwrite(&calcb->ccscb->ccscb_array[i * 2 + 1], sizeof(uint8_t), 1, file);
  }    

  fwrite(&calcb->n, sizeof(uint64_t), 1, file);
  fwrite(&calcb->e, sizeof(uint64_t), 1, file);
  fwrite(&calcb->nbb, sizeof(uint64_t), 1, file);

  for(i = 0; i < calcb->n + 2; i++)
    fwrite(&calcb->cd[i], sizeof(uint64_t), 1, file);
   
  for(i = 0; i < calcb->nbb; i++)
    fwrite(&calcb->adj[i], sizeof(uint8_t), 1, file);

  fclose(file);
}

/**
 * Reads a CompAdjListCB from a file.
 */
CompAdjListCB *read_CompAdjListCB(char *path)
{
  FILE *file = fopen(path, "rb");
  uint64_t i;
  CompCodes_CB *ccscb = calloc(1, sizeof(CompCodes_CB));
  CompAdjListCB *calcb = malloc(sizeof(CompAdjListCB));
  calcb->ccscb = ccscb;

  for(i = 0; i < NB_SECTIONS_CB; i++){
    assert(fread(&calcb->ccscb->ccscb_array[i * 2], sizeof(uint8_t), 1, file) == 1);
    if(calcb->ccscb->ccscb_array[i * 2] == ZETA_K_FUNC_ID)
      assert(fread(&calcb->ccscb->ccscb_array[i * 2 + 1], sizeof(uint8_t), 1, file) == 1);
  }

  assert(fread(&calcb->n, sizeof(uint64_t), 1, file) == 1);
  assert(fread(&calcb->e, sizeof(uint64_t), 1, file) == 1);
  assert(fread(&calcb->nbb, sizeof(uint64_t), 1, file) == 1);

  calcb->cd = malloc((calcb->n + 2) * sizeof(uint64_t));
  calcb->adj = malloc(calcb->nbb * sizeof(uint8_t));

  for (i = 0; i < calcb->n + 2; i++)
    assert(fread(&calcb->cd[i], sizeof(uint64_t), 1, file) == 1);

  for (i = 0; i < calcb->nbb; i++)
    assert(fread(&calcb->adj[i], sizeof(uint8_t), 1, file) == 1);

  fclose(file);
  return calcb;
}

/**
 * Decodes the list of successors of the node "node" and stores it in the array 
 * pointed to by the pointer "dest" and which has it's length stored in "*dlen".
 */
void decode_CompAdjListCB(CompAdjListCB *calcb, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...))
{
  *dlen=0;
  uint64_t u=calcb->cd[node];

  // first call
  if ( decoding_functions == NULL) {
    if(decoding_functions)
      free(decoding_functions);

    isfc = 1;

    decoding_functions = calloc(NB_SECTIONS_CB, sizeof(&unary_decoding));
    for(uint8_t i = 0; i < NB_SECTIONS_CB; i++){
      uint8_t tmp_ind = calcb->ccscb->ccscb_array[i * 2];
      if(decoding_functions[tmp_ind] == NULL) 
        decoding_functions[tmp_ind] = get_decoding_function(tmp_ind);
    }
  }

  if(u < calcb->cd[node + 1]){
     uint64_t
      ref = decoding_functions[calcb->ccscb->ccscb_struct.f_ref]
        (calcb->adj, &u, calcb->ccscb->ccscb_struct.ref_k),
      cpt=0,cptdecoded=0, 
      cptblocks=0, i=0,
      block_size, nb_blocks;

    //DECODING COPY BLOCKS
    if(ref>0) {
      decode_CompAdjListCB(calcb, node-ref, dest, dlen, 0,
        decoding_functions);

      uint8_t cv = 1;
      nb_blocks = decoding_functions[calcb->ccscb->ccscb_struct.f_bcnt]
        (calcb->adj, &u, calcb->ccscb->ccscb_struct.bcnt_k);

      while(cptblocks < nb_blocks - 1){
        block_size = decoding_functions[calcb->ccscb->ccscb_struct.f_cb]
          (calcb->adj, &u, calcb->ccscb->ccscb_struct.cb_k);
        if(cv){
          for(i = 0; i < block_size; i++){
            dest[cptdecoded++] = dest[cpt++];
          }
        }else{
          for(i = 0; i < block_size; i++)
            cpt++;
        }
        cv = !cv;
        cptblocks++;
      }
      uint64_t tmppos = *dlen - cpt;

      if(cv)
        for(i = 0; i < tmppos; i++){
          dest[cptdecoded++] = dest[cpt++];
        }
    }


    //DECODING THE LEFT OVER VALUES
    if(u < calcb->cd[node+1]){
      dest[cptdecoded++] = inv_v_func(node, decoding_functions[calcb->ccscb->ccscb_struct.f_res]
          (calcb->adj, &u, calcb->ccscb->ccscb_struct.res_k));
          
      while(u < calcb->cd[node + 1]){
        dest[cptdecoded] = dest[cptdecoded - 1] + decoding_functions[calcb->ccscb->ccscb_struct.f_res]
          (calcb->adj, &u, calcb->ccscb->ccscb_struct.res_k) + 1;
        cptdecoded++;
      }
    }
    *dlen=cptdecoded;
  }
   if (isfc)
    free(decoding_functions);
}


/**
 * Runs the Breadth-First Search algorithm on the CompAdjListCB
 * starting from the "curr".
 * Returns the array of the visited nodes, and stores it's length
 * in "*nbvals".
 */
uint64_t *bfs_CompAdjListCB(
  CompAdjListCB *calcb, uint64_t curr, uint64_t *nbvals)
{
  uint64_t v,
    s_file = 0, e_file = 0,
    *file = malloc((calcb->n + 1)*sizeof(uint64_t)),
    *dest = malloc((calcb->n + 1)*sizeof(uint64_t)),
    dlen = 0;
  uint8_t *seen = calloc(calcb->n + 1,sizeof(uint8_t));
  *nbvals = 0;
  
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...) = calloc(NB_SECTIONS_CB, sizeof(&unary_decoding));
  for(uint8_t i = 0; i < NB_SECTIONS_CB; i++){
    uint8_t tmp_ind = calcb->ccscb->ccscb_array[i * 2];
    if(decoding_functions[tmp_ind] == NULL) 
      decoding_functions[tmp_ind] = get_decoding_function(tmp_ind);
  }
  file[e_file++] = curr;
  seen[curr] = 1;
  
  for(uint64_t i = s_file; i < e_file; i++){   
    v = file[i];
    seen[v] = 1;
    decode_CompAdjListCB(calcb, v, dest, &dlen, 
      0, decoding_functions);
    for(uint64_t k = 0; k < dlen; k++){
      if(!seen[dest[k]]){
        file[e_file++] = dest[k];
        seen[dest[k]] = 1;
      }
    }
  }
  file = realloc(file, e_file * sizeof(uint64_t));
  *nbvals = e_file;
  free(decoding_functions);
  free(dest);
  free(seen);
  return file;
}
