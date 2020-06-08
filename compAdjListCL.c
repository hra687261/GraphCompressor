#include "compAdjListCL.h"

/**
 * Frees a CompAdjListCL and it's content
 */
void free_CompAdjListCL(CompAdjListCL *calcb)
{
  free(calcb->adj);
  free(calcb->cd);
  free(calcb);
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
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val),
  uint64_t expl_function(uint64_t val),
  uint64_t *u, uint8_t ws,
  uint8_t mrc)
{
  uint64_t i, j;
  CompAdjListCL *calcb = malloc(sizeof(CompAdjListCL));
  calcb->n = 0;
  calcb->e = 0;
  uint64_t v1, v2,
  //upper bound of the memory size we'll need to allocate (arbitrary)
  alloc_ub = 0,
    precv = 0,
    max_deg = 0,
    new_deg = 0;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //counting the max degree, the number of nodes and the number of edges
  FILE *file = fopen(path, "r");
  while (fscanf(file, "%lu %lu", &v1, &v2) == 2) {
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
    alloc_ub += expl_function(v2);
  }
  calcb->cd = calloc((calcb->n + 2), sizeof(uint64_t));
  calcb->adj = calloc((alloc_ub - 1) / 8 + 1, sizeof(uint8_t));

  uint64_t wlen = ws + 1;
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
    *window_succ = malloc(max_deg * wlen * sizeof(uint64_t)),
    *window_pos = malloc(max_deg * wlen * sizeof(uint64_t)),
    *window_lens = calloc(wlen, sizeof(uint64_t)),
    *window_nodes = calloc(wlen, sizeof(uint64_t)),

    c_node = 0, it_node,
    tmppos_2, nb_v_tocode,

    // copy list the list of copied nodes from the referenced list of successors
    *copy_list = calloc(max_deg, sizeof(uint64_t));

  uint8_t
    /*
    contains the legnth of the reference chain of each 
    ex : (15 references 12 which references 5 -> ref_cnts[5]=0, ref_cnts[12]=1, ref_cnts[15]=2  )
    */
    *ref_cnts = calloc(calcb->n + 1, sizeof(uint8_t));

  // the reference (c_node-ref : the referred node)
  uint64_t ref = 0;

  // the number of values that were compressed with the reference technique
  uint64_t nbv_refc = 0;

  // the last node that had it's successors list compressed
  uint64_t precun = 0;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // compressing the edge list
  rewind(file);
  for (uint64_t ind = 0; ind < calcb->e + 1; ind++) {
    if (ind < calcb->e)
      assert(fscanf(file, "%lu %lu", &v1, &v2) == 2);

    if (v1 == window_nodes[c_w_ind] &&
      ind > 0 &&
      ind < calcb->e) {
      tmppos_2 = c_w_ind * max_deg + window_lens[c_w_ind];
      window_succ[tmppos_2] = v2;
      window_pos[tmppos_2] = window_lens[c_w_ind];
      window_lens[c_w_ind]++;
    } else {
      it_w_ind = (c_w_ind + 1) % wlen;
      c_node = window_nodes[c_w_ind];
      it_node = window_nodes[it_w_ind];

      nbv_refc = 0;
      ref = 0;

      uint64_t
        *c_arr = window_succ + max_deg * c_w_ind,
        *ca_pos = window_pos + max_deg * c_w_ind,
        ccalcben = window_lens[c_w_ind];

      if (ccalcben > 0) {

        // choosing the reference list (the one which has the most nodes in common with the list of successors of the current node)
        while (it_w_ind != c_w_ind &&
          ref_cnts[it_node] < mrc) {
          uint64_t
            *
            it_arr = window_succ + it_w_ind * max_deg,
            it_len = window_lens[it_w_ind];

          nb_v_tocode = 0;
          i = 0;
          j = 0;
          while (i < it_len && j < ccalcben)
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
          it_w_ind = (it_w_ind + 1) % wlen;
          it_node = window_nodes[it_w_ind];
        }

        //writing the reference
        encoding_function(calcb->adj, u, ref);

        if (ref > 0) {
          //updating the ref count
          ref_cnts[c_node] = ref_cnts[window_nodes[ref_w_ind]] + 1;

          uint64_t
          p1 = 0, p2 = nbv_refc,

            ra_it = 0,
            ca_it = 0,

            // the referenced list of successors (sorted in incremental order)
            * r_arr = window_succ + max_deg * ref_w_ind,

            // the real positions of the nodes in the referenced list of successors after compression (if the list was compressed)
            * ra_pos = window_pos + max_deg * ref_w_ind,

            // the length of the referenced list 
            rcalcben = window_lens[ref_w_ind];

          // building the copy_list
          while (ra_it < rcalcben && ca_it < ccalcben) {
            if (r_arr[ra_it] == c_arr[ca_it]) {
              ca_pos[ca_it++] = p1++;
              copy_list[ra_pos[ra_it++]] = 1;
            } else if (r_arr[ra_it] > c_arr[ca_it]) {
              ca_pos[ca_it++] = p2++;
            } else {
              ra_it++;
            }
          }


          ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          uint64_t cpt = 0;
          for (i = 0; i < rcalcben; i++) {
            if (copy_list[i]) {
              cpt += copy_list[i];
              copy_list[i] = cpt;
            }
          }
          ra_it = 0;
          ca_it = 0;
          while (ra_it < rcalcben && ca_it < ccalcben) {
            if (r_arr[ra_it] == c_arr[ca_it]) {
              ca_pos[ca_it] = copy_list[ra_pos[ra_it]] - 1;
              copy_list[ra_pos[ra_it++]] = 1;
            } else if (r_arr[ra_it] > c_arr[ca_it]) {
              ca_it++;
            } else {
              ra_it++;
            }
          }

          /*
          writing the copy list 
          */
          for (i = 0; i < rcalcben; i++) {
            if (copy_list[i]) {
              set_nth_bit(calcb->adj, u, *u);
              copy_list[i] = 0;
            } else
              clear_nth_bit(calcb->adj, u, *u);
          }
        }

        /*
        the left over values are stored in [nbv_refc .. ccalcben[
        in increasing order
        */
        // writing the leftover values, the ones that weren't compressed
        for (i = 0; i < ccalcben; i++) {
          if (ca_pos[i] > nbv_refc) {
            encoding_function(calcb->adj, u,
              c_arr[i] - precv - 1);
            precv = c_arr[i];
            continue;
          }
          //the first left over node
          if (ca_pos[i] == nbv_refc) {
            precv = c_arr[i];
            encoding_function(calcb->adj, u,
              func(c_node, precv));
          }
        }
      }

      // storing the starting bit adress of the next node
      calcb->cd[c_node + 1] = *u;
      // if there are nodes that don't have successors, their starting bit is address is equal to the  
      // ending bit address of the last node that has it's list of successors coded
      if (precun > 0 && calcb->cd[c_node] == 0) {
        for (i = precun + 1; i <= c_node; i++)
          calcb->cd[i] = calcb->cd[precun];
      }
      precun = c_node + 1;
      c_w_ind = (c_w_ind + 1) % wlen;
      window_nodes[c_w_ind] = v1;

      tmppos_2 = c_w_ind * max_deg;
      window_succ[tmppos_2] = v2;
      window_pos[tmppos_2] = 0;
      window_lens[c_w_ind] = 1;
    }
  }

  if (calcb->n + 1 > precun)
    for (i = precun; i <= calcb->n + 1; i++)
      calcb->cd[i] = *u;

  calcb->nbb = (( *u) / 8 + 1);
  calcb->adj = realloc(calcb->adj, calcb->nbb * sizeof(uint8_t));

  fclose(file);
  free(window_succ);
  free(window_pos);
  free(window_lens);
  free(window_nodes);
  free(copy_list);
  free(ref_cnts);

  return calcb;
}

/**
 * Decodes the list of successors of the node "node" and stores them in the array 
 * pointed to by the pointer "dest" and which has it's length stored in "*dlen".
 */
void decode_CompAdjListCL(CompAdjListCL *calcb, uint64_t node,
  uint64_t *dest, uint64_t *dlen,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s))
{
  *dlen = 0;
  uint64_t u = calcb->cd[node];

  if (u < calcb->cd[node + 1]) {
    uint64_t
    ref = decoding_function(calcb->adj, &u),
      cpt = 0;

    if (ref > 0) {
      decode_CompAdjListCL(calcb, node - ref, dest, dlen, decoding_function);
      for (uint64_t i = 0; i < * dlen; i++)
        if (read_nth_bit(calcb->adj, u++))
          dest[cpt++] = dest[i];
    }

    if (u < calcb->cd[node + 1]) {
      dest[cpt++] = inv_func(node, decoding_function(calcb->adj, &u));
      while (u < calcb->cd[node + 1]) {
        dest[cpt] = dest[cpt - 1] + decoding_function(calcb->adj, &u) + 1;
        cpt++;
      }
    }
    * dlen = cpt;
  }
}

/**
 * Stores a CompAdjListCL in a file.
 */
void write_CompAdjListCL(CompAdjListCL *calcb, uint8_t id, char *path)
{
  FILE *file = fopen(path, "wb");
  uint64_t i;

  fwrite( & id, sizeof(uint8_t), 1, file);
  if (id == 4) {
    uint8_t k = getK();
    fwrite( & k, sizeof(uint8_t), 1, file);
  }

  fwrite( & calcb->n, sizeof(uint64_t), 1, file);
  fwrite( & calcb->e, sizeof(uint64_t), 1, file);
  fwrite( & calcb->nbb, sizeof(uint64_t), 1, file);

  for (i = 0; i < calcb->n + 2; i++)
    fwrite( & calcb->cd[i], sizeof(uint64_t), 1, file);

  for (i = 0; i < calcb->nbb; i++)
    fwrite( & calcb->adj[i], sizeof(uint8_t), 1, file);

  fclose(file);
}

/**
 * Reads a CompAdjListCL from a file.
 * 
 * id: pointer to a uint8_t, in which the id of the function that was used 
 *     to encode the successor lists of the CompAdjListCB will be stored.
 */
CompAdjListCL *read_CompAdjListCL(char *path, uint8_t *id)
{
  FILE *file = fopen(path, "rb");
  uint64_t i;
  uint8_t k = 0;
  CompAdjListCL *calcb = malloc(sizeof(CompAdjListCL));

  assert(fread(id, sizeof(uint8_t), 1, file) == 1);
  if ( *id == 4) {
    assert(fread( & k, sizeof(uint8_t), 1, file) == 1);
    setK(k);
  }

  assert(fread( & calcb->n, sizeof(uint64_t), 1, file) == 1);
  assert(fread( & calcb->e, sizeof(uint64_t), 1, file) == 1);
  assert(fread( & calcb->nbb, sizeof(uint64_t), 1, file) == 1);

  calcb->cd = malloc((calcb->n + 2) * sizeof(uint64_t));
  calcb->adj = malloc(calcb->nbb * sizeof(uint8_t));

  for (i = 0; i < calcb->n + 2; i++)
    assert(fread( &calcb->cd[i], sizeof(uint64_t), 1, file) == 1);

  for (i = 0; i < calcb->nbb; i++)
    assert(fread( &calcb->adj[i], sizeof(uint8_t), 1, file) == 1);

  fclose(file);
  return calcb;
}

/**
 * Runs the Breadth-First Search algorithm on the graph stored in the CompAdjListCL
 * starting from the node "curr", 
 */
uint64_t *bfs_CompAdjListCL(
  CompAdjListCL *calcb, uint64_t curr, uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s))
{
  uint64_t v,
  s_file = 0, e_file = 0,
    *file = malloc((calcb->n + 1) * sizeof(uint64_t)),
    *dest = malloc((calcb->n + 1) * sizeof(uint64_t)),
    dlen = 0;
  *nbvals = 0;

  uint8_t *seen = calloc(calcb->n + 1, sizeof(uint8_t));

  file[e_file++] = curr;
  seen[curr] = 1;

  for (uint64_t i = s_file; i < e_file; i++) {
    v = file[i];
    seen[v] = 1;
    dlen = 0;
    decode_CompAdjListCL(calcb, v, dest, &dlen, decoding_function);
    for (uint64_t k = 0; k < dlen; k++)
      if (!seen[dest[k]]) {
        file[e_file++] = dest[k];
        seen[dest[k]] = 1;
      }
  }
  file = realloc(file, e_file * sizeof(uint64_t));
  *nbvals = e_file;
  free(dest);
  free(seen);
  return file;
}