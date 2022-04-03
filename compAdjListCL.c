#include "compAdjListCL.h"

/**
 * Frees a CompAdjListCL and it's content.
 */
void free_CompAdjListCL(CompAdjListCL *calcl)
{
  free(calcl->ccscl);
  free(calcl->adj);
  free(calcl->cd);
  free(calcl);
}

/**
 * Loads a CompAdjListCL from a file containing an edgelist,
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
CompAdjListCL *load_CompAdjListCL(char *path,
  CompCodes_CL *ccscl, uint64_t *u,
  uint8_t ws, uint8_t mrc)
{
 uint64_t i, j;
  CompAdjListCL *calcl = malloc(sizeof(CompAdjListCL));
  calcl->n = 0;
  calcl->e = 0;
  calcl->ccscl = ccscl;

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
    calcl->n = max3(calcl->n, v1, v2);
    calcl->e++;
  }
  if (new_deg > max_deg)
    max_deg = new_deg;

  if(calcl->e == 0) {
    fclose(file);
    return calcl;
  }   

  alloc_ub = ((calcl->e + 1) * UB_BITS_PER_LINK_CL) / 8 + 1;
  calcl->cd = calloc(calcl->n + 2, sizeof(uint64_t));
  calcl->adj = calloc(alloc_ub, sizeof(uint8_t));

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
    
    c_node=0, it_node, tmppos_2, nb_v_tocode,

    // copy list the list of copied nodes from the referenced list of successors
    *copy_list = calloc(max_deg, sizeof(uint64_t));

  //  contains the legnth of the reference chain of each list of successors
  //    ex : 
  //    if 15 references 12 which references 5 witch doesn't refence any other node then :
  //    ref_cnts[5]=0, ref_cnts[12]=1, ref_cnts[15]=2 
  uint8_t *ref_cnts = calloc(calcl->n + 1, sizeof(uint8_t)),
    
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
    = calloc(NB_CODES, sizeof(&unary_encoding));

  for(i = 0; i < NB_SECTIONS_CL; i++){
    uint8_t tmp_ind = ccscl->ccscl_array[i*2];
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
  for (uint64_t ind = 1; ind < calcl->e + 1; ind++) {
    if (ind < calcl->e){
      assert(fscanf(file, "%lu %lu", & v1, & v2) == 2);
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
      calcl->cd[i] = *u;
    
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
      
      uint64_t ra_len = 0;
     
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
      } 
        


      precv = window_succ[max_deg * c_w_ind];
      
      // writing the reference
      encoding_functions[ccscl->ccscl_struct.f_ref](calcl->adj, u, ref, ccscl->ccscl_struct.ref_k);


      // Writing the copy list
      for (i = 0; i < ra_len; i++) {
        if (copy_list[i]) {
          SET_BIT(*(uint8_t *) &calcl->adj[*u / 8], *u % 8);
          (*u)++;
          copy_list[i] = 0;
        } else {
          CLR_BIT(*(uint8_t *)  &calcl->adj[*u / 8], *u % 8);
          (*u)++;
        }
      }

      // writing the residual values, the ones that weren't compressed
      for (i = 0; i < ca_len; i++) {
        coded[i] = 0;
        if (ca_pos[i] > nbv_refc ) {
          encoding_functions[ccscl->ccscl_struct.f_res](calcl->adj, u, c_arr[i] - precv - 1, ccscl->ccscl_struct.res_k);
          precv = c_arr[i];
          continue;
        }
        // writing the first residual node
        if (ca_pos[i] == nbv_refc ) {
          precv = c_arr[i];
          encoding_functions[ccscl->ccscl_struct.f_res](calcl->adj, u, v_func(c_node, precv), ccscl->ccscl_struct.res_k);
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
  for(i = precun + 1; i <= calcl->n + 1; i++)
    calcl->cd[i] = *u;

  calcl->nbb = ((*u - 1) / 8 + 1);
  calcl->adj = realloc(calcl->adj, calcl->nbb * sizeof(uint8_t));

  fclose(file);
  free(encoding_functions);
  free(window_succ);
  free(window_pos);
  free(window_lens);
  free(window_nodes);
  free(copy_list);
  free(coded);
  free(ref_cnts);

  return calcl;
}

/**
 * Stores a CompAdjListCL in a file.
 */
void write_CompAdjListCL(CompAdjListCL *calcl, char *path)
{
  FILE *file=fopen(path,"wb");
  uint64_t i;

  for(i=0;i<NB_SECTIONS_CL;i++){
    uint64_t tmp = calcl->ccscl->ccscl_array[i * 2];
    fwrite(&tmp, sizeof(uint8_t), 1, file);
    if(tmp == ZETA_K_FUNC_ID)
      fwrite(&calcl->ccscl->ccscl_array[i * 2 + 1], sizeof(uint8_t), 1, file);
  }    

  fwrite(&calcl->n, sizeof(uint64_t), 1, file);
  fwrite(&calcl->e, sizeof(uint64_t), 1, file);
  fwrite(&calcl->nbb, sizeof(uint64_t), 1, file);

  for(i=0;i<calcl->n+2;i++)
    fwrite(&calcl->cd[i], sizeof(uint64_t), 1, file);
   
  for(i=0;i<calcl->nbb;i++)
    fwrite(&calcl->adj[i], sizeof(uint8_t), 1, file);

  fclose(file);
}

/**
 * Reads a CompAdjListCL from a file.
 */
CompAdjListCL *read_CompAdjListCL(char *path)
{
  FILE *file = fopen(path, "rb");
  uint64_t i;
  CompCodes_CL *ccscl = calloc(1,sizeof(CompCodes_CL));
  CompAdjListCL *calcl = malloc(sizeof(CompAdjListCL));
  calcl->ccscl = ccscl;

  for(i = 0; i < NB_SECTIONS_CL; i++){
    assert(fread(&calcl->ccscl->ccscl_array[i * 2], sizeof(uint8_t), 1, file) == 1);
    if(calcl->ccscl->ccscl_array[i * 2] == ZETA_K_FUNC_ID)
      assert(fread(&calcl->ccscl->ccscl_array[i * 2 + 1], sizeof(uint8_t), 1, file) == 1);
  }

  assert(fread(&calcl->n, sizeof(uint64_t), 1, file) == 1);
  assert(fread(&calcl->e, sizeof(uint64_t), 1, file) == 1);
  assert(fread(&calcl->nbb, sizeof(uint64_t), 1, file) == 1);

  calcl->cd = malloc((calcl->n + 2) * sizeof(uint64_t));
  calcl->adj = malloc(calcl->nbb * sizeof(uint8_t));

  for (i = 0; i < calcl->n + 2; i++)
    assert(fread(&calcl->cd[i], sizeof(uint64_t), 1, file) == 1);

  for (i = 0; i < calcl->nbb; i++)
    assert(fread(&calcl->adj[i], sizeof(uint8_t), 1, file) == 1);

  fclose(file);
  return calcl; 
}

/**
 * Decodes the list of successors of the node "node" and stores it in the array 
 * pointed to by the pointer "dest" and which has it's length stored in "*dlen".
 */
void decode_CompAdjListCL(CompAdjListCL *calcl, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...))
{
  *dlen = 0;
  uint64_t u = calcl->cd[node];

  // first call
  if (decoding_functions == NULL) {
    if(decoding_functions)
      free(decoding_functions);

    isfc = 1;

    decoding_functions = calloc(NB_SECTIONS_CL, sizeof(&unary_decoding));
    for(uint8_t i = 0; i < NB_SECTIONS_CL; i++){
      uint8_t tmp_ind = calcl->ccscl->ccscl_array[i * 2];
      if(decoding_functions[tmp_ind] == NULL) 
        decoding_functions[tmp_ind] = get_decoding_function(tmp_ind);
    }
  }

  if (u < calcl->cd[node + 1]) {
    uint64_t
    ref = decoding_functions[calcl->ccscl->ccscl_struct.f_ref]
        (calcl->adj, &u, calcl->ccscl->ccscl_struct.ref_k),
    cptdecoded = 0;

    if (ref > 0) {
      decode_CompAdjListCL(calcl, node - ref, dest, dlen, 0, decoding_functions);
      for (uint64_t i = 0; i < * dlen; i++)
        if (CHECK_BIT(*(uint8_t *) &calcl->adj[u / 8], u % 8)){
          dest[cptdecoded++] = dest[i];
          u++;
        }
    }

    //DECODING THE LEFT OVER VALUES
    if (u < calcl->cd[node + 1]) {
      dest[cptdecoded++] = inv_v_func(node, decoding_functions[calcl->ccscl->ccscl_struct.f_res]
          (calcl->adj, &u, calcl->ccscl->ccscl_struct.res_k));
      while (u < calcl->cd[node + 1]) {
        dest[cptdecoded] = dest[cptdecoded - 1] + decoding_functions[calcl->ccscl->ccscl_struct.f_res]
          (calcl->adj, &u, calcl->ccscl->ccscl_struct.res_k) + 1;
        cptdecoded++;
      }
    }
    *dlen = cptdecoded;
  }
  if (isfc)
    free(decoding_functions);
}

/**
 * Runs the Breadth-First Search algorithm on the CompAdjListCL
 * starting from the "curr".
 * Returns the array of the visited nodes, and stores it's length
 * in "*nbvals".
 */
uint64_t *bfs_CompAdjListCL(
  CompAdjListCL *calcl, uint64_t curr, uint64_t *nbvals)
{
  uint64_t v,
    s_file = 0, e_file = 0,
    *file = malloc((calcl->n + 1) * sizeof(uint64_t)),
    *dest = malloc((calcl->n + 1) * sizeof(uint64_t)),
    dlen = 0;
  uint8_t *seen = calloc(calcl->n + 1, sizeof(uint8_t));
  *nbvals = 0;
  
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...) = calloc(NB_SECTIONS_CL, sizeof(&unary_decoding));
  for(uint8_t i = 0; i < NB_SECTIONS_CL; i++){
    uint8_t tmp_ind = calcl->ccscl->ccscl_array[i * 2];
    if(decoding_functions[tmp_ind] == NULL) 
      decoding_functions[tmp_ind] = get_decoding_function(tmp_ind);
  }
  file[e_file++]=curr;
  seen[curr]=1;
  
  for(uint64_t i = s_file; i < e_file; i++){   
    v = file[i];
    seen[v] = 1;
    decode_CompAdjListCL(calcl, v, dest, &dlen, 
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