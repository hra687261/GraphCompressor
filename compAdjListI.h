#ifndef ADJLISTI
#define ADJLISTI


#include "integerEncoding.h"

typedef struct _CompAdjListI{
	uint64_t n; // number of nodes
	uint64_t e; // number of edges
  uint64_t md; // maximum degree of nodes

  uint64_t *cd; // n's neighbours are in [ cd[n] .. cd[n+1] [
  uint8_t *adj;
  uint64_t nbb; //nb bytes used by adj

  uint8_t th; // threshold
}CompAdjListI;

void free_CompAdjListI(CompAdjListI *alg);

CompAdjListI *load_CompAdjListI(char *path, uint8_t ign, 
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val), 
  uint64_t expl_function(uint64_t val), 
  uint64_t *u, uint8_t ws, 
  uint8_t mrc, uint8_t threshold);

void decode_CompAdjListI(CompAdjListI *alg, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s),
  uint64_t m_nb_intrs,uint64_t *intrvs);

void write_CompAdjListI(CompAdjListI *alg, uint8_t id, char *path);

CompAdjListI *read_CompAdjListI(char *path, uint8_t *id);

uint64_t *bfs_CompAdjListI(
  CompAdjListI *alg,uint64_t curr, uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s));


#endif