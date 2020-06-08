#include "compAdjListG.h"


/**
 * Frees the allocated memory for a CompAdjListG
 */
void free_CompAdjListG(CompAdjListG *calg)
{
  free(calg->adj);
  free(calg->cd);
  free(calg);
}

/**
 * Reads a file containing an edgelist
 * And loads it into a CompAdjListG 
 */
CompAdjListG *load_CompAdjListG(char *path,
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u , uint64_t val),
  uint64_t expl_function(uint64_t val), uint64_t *u )
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

  FILE *file = fopen(path, "r");
  while (fscanf(file, "%lu %lu", &v1, &v2) == 2) {
    calg->n = max3(calg->n, v1, v2);
    calg->e++;
  }

  // cd[i+1]-cd[i]= size of i's neighbours in number of bits
  calg->cd = calloc((calg->n + 2), sizeof(uint64_t));
  d = calloc((calg->n + 1), sizeof(uint64_t));
  prec = calloc((calg->n + 1), sizeof(uint64_t));

  rewind(file);
  while (fscanf(file, "%lu %lu", &v1, &v2) == 2) {
    // counting the size of every node's list of neighbours
    // by summing the expected length of it's neighbours

    if (prec[v1] == 0) {
      v = func(v1, v2);
    } else {
      // this new neighbour of v1 needs to be bigger than it's previous one 
      // (the file needs to be sorted by the 1st and 2nd column) 
      assert(v2 > prec[v1]);
      v = v2 - prec[v1] - 1;
    }
    prec[v1] = v2;
    //adding the expected length of v to the size of the neighbours of v1
    d[v1] += expl_function(v);

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
      v = func(v1, v2);
    } else {
      v = v2 - prec[v1] - 1;
    }
    prec[v1] = v2;
    encoding_function(calg->adj, u, v);
    d[v1] += *u - tmp;

  }

  fclose(file);
  free(prec);
  free(d);
  return calg;
}

void write_CompAdjListG(CompAdjListG *calg, uint8_t id, char *path)
{
  FILE *file = fopen(path, "wb");
  uint64_t i;

  fwrite( &id, sizeof(char), 1, file);
  if (id == 4) {
    uint8_t k = getK();
    fwrite( &k, sizeof(char), 1, file);
  }

  fwrite( &calg->n, sizeof(uint64_t), 1, file);
  fwrite( &calg->e, sizeof(uint64_t), 1, file);
  fwrite( &calg->nbb, sizeof(uint64_t), 1, file);

  for (i = 0; i < calg->n + 2; i++)
    fwrite( &calg->cd[i], sizeof(uint64_t), 1, file);

  for (i = 0; i < calg->nbb; i++)
    fwrite( &calg->adj[i], sizeof(uint8_t), 1, file);

  fclose(file);
}

CompAdjListG *read_CompAdjListG(char *path, uint8_t *id)
{
  FILE *file = fopen(path, "rb");
  uint64_t i, k;
  CompAdjListG *calg = malloc(sizeof(CompAdjListG));

  assert(fread(id, sizeof(uint8_t), 1, file) == 1);
  if ( *id == 4) {
    assert(fread( &k, sizeof(uint8_t), 1, file) == 1);
    setK(k);
  }

  assert(fread( &calg->n, sizeof(uint64_t), 1, file) == 1);
  assert(fread( &calg->e, sizeof(uint64_t), 1, file) == 1);
  assert(fread( &calg->nbb, sizeof(uint64_t), 1, file) == 1);

  calg->cd = malloc((calg->n + 2) * sizeof(uint64_t));
  calg->adj = malloc(calg->nbb * sizeof(uint8_t));

  for (i = 0; i < calg->n + 2; i++)
    assert(fread( &calg->cd[i], sizeof(uint64_t), 1, file) == 1);

  for (i = 0; i < calg->nbb; i++)
    assert(fread( &calg->adj[i], sizeof(uint8_t), 1, file) == 1);

  fclose(file);
  return calg;
}

uint64_t *bfs_CompAdjListG(CompAdjListG *calg, uint64_t curr,
  uint8_t *seen, uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s))
{
  uint64_t i, v1, v2, u,
  s_file = 0, e_file = 0,
    *file = malloc((calg->n + 1) * sizeof(uint64_t));
  uint8_t sv;
  *nbvals = 0;

  if (seen == NULL) {
    seen = calloc(calg->n + 1, sizeof(uint8_t));
  } else
  if (seen[curr] == 1)
    return NULL;

  file[e_file++] = curr;
  seen[curr] = 1;

  for (i = s_file; i < e_file; i++) {
    v1 = file[i];
    seen[v1] = 1;
    u = calg->cd[v1];
    v2 = 0;
    sv = 1;
    while (u < calg->cd[v1 + 1]) {
      if (sv) {
        v2 = inv_func(v1, decoding_function(calg->adj, & u));
        sv = 0;
      } else
        v2 = v2 + 1 + decoding_function(calg->adj, & u);

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