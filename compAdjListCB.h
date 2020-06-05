#ifndef ADJLISTCB
#define ADJLISTCB

#include "integerEncoding.h"

typedef struct _CompAdjListCB{
	uint64_t n; // number of nodes
	uint64_t e; // number of edges
  
  uint64_t *cd; // n's neighbours are in [ cd[n] .. cd[n+1] [
  uint8_t *adj;
  uint64_t nbb; //nb bytes used by adj

}CompAdjListCB;

void free_CompAdjListCB(CompAdjListCB *alg);

CompAdjListCB *load_CompAdjListCB(char *path, uint8_t ign, 
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val), 
  uint64_t expl_function(uint64_t val), 
  uint64_t *u, uint8_t ws, 
  uint8_t mrc);

void print_CompAdjListCB(CompAdjListCB *alg, 
  uint64_t decoding_function(uint8_t *arr,uint64_t *s));

void decode_CompAdjListCB(CompAdjListCB *alg, uint64_t node, 
  uint64_t *dest, uint64_t *dlen,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s));

void write_CompAdjListCB(CompAdjListCB *alg, uint8_t id, char *path);

CompAdjListCB *read_CompAdjListCB(char *path, uint8_t *id);

uint64_t *bfs_CompAdjListCB(
  CompAdjListCB *alg,uint64_t curr,
  uint8_t *seen,uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s));

#endif