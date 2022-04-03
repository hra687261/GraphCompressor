#include "compAdjListG.h"


/**
 * Frees the allocated memory for a CompAdjListG.
 */
void free_CompAdjListG(CompAdjListG *calg)
{
  free(calg->adj);
  free(calg->cd);
  free(calg);
}


/**
 * Loads a CompAdjListG from a file containing an edgelist,
 * "path" is the path to the file, the file must contain on every line 
 * a pair of positive numbers x y, separated by a space, in which x->y is a link,
 * the list needs to be sorted in increasing order by the first, then the 
 * second column.
 * 
 * "*u" points to first bit after the last used bit for the array of 
 * compressed successor lists.
 * f_id: code id for the function used to code the successor lists.
 * k: shrinking parameter, if the code if the zeta code, 0 otherwise.
 */
CompAdjListG *load_CompAdjListG(char *path, 
  uint8_t f_id, uint8_t k, uint64_t *u)
{
  uint64_t
  v1, v2, i, v,
  *d, // d[i] : the total size in number of bits of the coded values of i's neighbours
  *prec; // prec[i] : the last neighbour of node i that was added to the adjlist
  // since the adjlist needs to be sorted so for us to apply the gaps data compression technique
  // we suppose that the file was pre processed
  // if it wasn't then an assertion error will be raised during the execution
  CompAdjListG *calg = malloc(sizeof(CompAdjListG));
  calg->n = 0;
  calg->e = 0;
  calg->f_id = f_id;
  calg->k = k;

  FILE *file = fopen(path, "r");
  while (fscanf(file, "%lu %lu", &v1, &v2) == 2) {
    calg->n = max3(calg->n, v1, v2);
    calg->e++;
  }

  // cd[i+1]-cd[i]= size of i's neighbours in number of bits
  calg->cd = calloc((calg->n + 2), sizeof(uint64_t));
  d = calloc((calg->n + 1), sizeof(uint64_t));
  prec = calloc((calg->n + 1), sizeof(uint64_t));

  uint64_t (*expl_function)(uint64_t val, ...)
    = get_expl_function(calg->f_id);
  void (*encoding_function)(uint8_t *arr, uint64_t *u, uint64_t val, ...)
    = get_encoding_function(calg->f_id);

  rewind(file);
  while (fscanf(file, "%lu %lu", &v1, &v2) == 2) {
    // counting the size of every node's list of neighbours
    // by summing the expected length of it's neighbours

    if (prec[v1] == 0) {
      v = v_func(v1, v2);
    } else {
      // this new neighbour of v1 needs to be bigger than it's previous one 
      // (the file needs to be sorted by the 1st and 2nd column) 
      assert(v2 > prec[v1]);
      v = v2 - prec[v1] - 1;
    }
    prec[v1] = v2;
    //adding the expected length of v to the size of the neighbours of v1
    d[v1] += expl_function(v, calg->k);

  }
  calg->cd[0] = 0;
  for (i = 1; i < calg->n + 2; i++) {
    calg->cd[i] = calg->cd[i - 1] + d[i - 1];
    d[i - 1] = 0;
    prec[i - 1] = 0;
  }

  calg->nbb = (calg->cd[calg->n + 1] - 1) / 8 + 1;
  calg->adj = calloc(calg->nbb, sizeof(uint8_t));

  *u = 0;
  rewind(file);
  while (fscanf(file, "%lu %lu", &v1, &v2) == 2) {
    // counting the value of the neighbour of each node using the gaps technique
    // and encoding that value into the neighbours array 

    *u = calg->cd[v1] + d[v1];
    uint64_t tmp = *u;
    if (prec[v1] == 0) {
      v = v_func(v1, v2);
    } else {
      v = v2 - prec[v1] - 1;
    }
    prec[v1] = v2;
    encoding_function(calg->adj, u, v, calg->k);
    d[v1] += *u - tmp;

  }

  fclose(file);
  free(prec);
  free(d);
  return calg;
}

/**
 * Stores a CompAdjListG in a file.
 */
void write_CompAdjListG(CompAdjListG *calg, char *path)
{
  FILE *file = fopen(path, "wb");
  uint64_t i;

  fwrite(&calg->f_id, sizeof(uint8_t), 1, file);
  if (calg->f_id == ZETA_K_FUNC_ID) 
    fwrite(&calg->k, sizeof(uint8_t), 1, file);

  fwrite(&calg->n, sizeof(uint64_t), 1, file);
  fwrite(&calg->e, sizeof(uint64_t), 1, file);
  fwrite(&calg->nbb, sizeof(uint64_t), 1, file);

  for (i = 0; i < calg->n + 2; i++)
    fwrite(&calg->cd[i], sizeof(uint64_t), 1, file);

  for (i = 0; i < calg->nbb; i++)
    fwrite(&calg->adj[i], sizeof(uint8_t), 1, file);

  fclose(file);
}

/**
 * Reads a CompAdjListG from a file.
 */
CompAdjListG *read_CompAdjListG(char *path)
{
  FILE *file = fopen(path, "rb");
  uint64_t i;
  CompAdjListG *calg = malloc(sizeof(CompAdjListG));

  assert(fread(&calg->f_id, sizeof(uint8_t), 1, file) == 1);
  if (calg->f_id == ZETA_K_FUNC_ID) 
    assert(fread(&calg->k, sizeof(uint8_t), 1, file) == 1);

  assert(fread(&calg->n, sizeof(uint64_t), 1, file) == 1);
  assert(fread(&calg->e, sizeof(uint64_t), 1, file) == 1);
  assert(fread(&calg->nbb, sizeof(uint64_t), 1, file) == 1);

  calg->cd = malloc((calg->n + 2) * sizeof(uint64_t));
  calg->adj = malloc(calg->nbb * sizeof(uint8_t));

  for (i = 0; i < calg->n + 2; i++)
    assert(fread(&calg->cd[i], sizeof(uint64_t), 1, file) == 1);

  for (i = 0; i < calg->nbb; i++)
    assert(fread(&calg->adj[i], sizeof(uint8_t), 1, file) == 1);

  fclose(file);
  return calg;
}

/**
 * Runs the Breadth-First Search algorithm on the CompAdjListG
 * starting from the "curr".
 * Returns the array of the visited nodes, and stores it's length
 * in "*nbvals".
 */
uint64_t *bfs_CompAdjListG(
  CompAdjListG *calg, uint64_t curr, uint64_t *nbvals)
{
  uint64_t i, v1, v2, u,
  s_file = 0, e_file = 0,
    *file = malloc((calg->n + 1) * sizeof(uint64_t));
  uint8_t sv;
  *nbvals = 0;

  uint8_t *seen = calloc(calg->n + 1, sizeof(uint8_t));

  file[e_file++] = curr;
  seen[curr] = 1;


  uint64_t (*decoding_function)(uint8_t *arr, uint64_t *s, ...)
    = get_decoding_function(calg->f_id);

  for (i = s_file; i < e_file; i++) {
    v1 = file[i];
    seen[v1] = 1;
    u = calg->cd[v1];
    v2 = 0;
    sv = 1;
    while (u < calg->cd[v1 + 1]) {
      if (sv) {
        v2 = inv_v_func(v1, decoding_function(calg->adj, &u, calg->k));
        sv = 0;
      } else
        v2 = v2 + 1 + decoding_function(calg->adj, &u, calg->k);

      if (!seen[v2] && v2 != v1) {
        file[e_file++] = v2;
        seen[v2] = 1;
      }
    }
  }
  free(seen);
  file = realloc(file, e_file * sizeof(uint64_t));
  * nbvals = e_file;
  return file; 
}