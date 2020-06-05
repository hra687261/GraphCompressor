#ifndef ADJLISTCP
#define ADJLISTCP

#include "integerEncoding.h"

typedef struct _CompAdjListCL{
	uint64_t n; // number of nodes
	uint64_t e; // number of edges
 
  uint64_t *cd; // n's neighbours are in [ cd[n] .. cd[n+1] [
  uint8_t *adj;
  uint64_t nbb; //nb bytes used by adj

}CompAdjListCL;

void free_CompAdjListCL(CompAdjListCL *alg);

CompAdjListCL *load_CompAdjListCL(char *path, uint8_t ign, 
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val), 
  uint64_t expl_function(uint64_t val), 
  uint64_t *u, uint8_t ws, 
  uint8_t mrc);

void print_CompAdjListCL(CompAdjListCL *alg, 
  uint64_t decoding_function(uint8_t *arr,uint64_t *s));

void decode_CompAdjListCL(CompAdjListCL *alg, uint64_t node, 
  uint64_t *dest, uint64_t *dlen,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s));

void write_CompAdjListCL(CompAdjListCL *alg, uint8_t id, char *path);

CompAdjListCL *read_CompAdjListCL(char *path, uint8_t *id);

uint64_t *bfs_CompAdjListCL(
  CompAdjListCL *alg,uint64_t curr,
  uint8_t *seen,uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s));
    
    
  
#endif 