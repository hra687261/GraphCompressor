#ifndef ADJLISTGAPS
#define ADJLISTGAPS

#include "integerEncoding.h"

/**
 * Upper bound of the number of bits per link of the resulting compressed graph.
 */
#define UB_BITS_PER_LINK_G 32

typedef struct _compadjlistG{
	uint64_t n;   // number of nodes
	uint64_t e;   // number of edges
  
  uint64_t *cd; // n's neighbours are in [ cd[n] .. cd[n+1] [
  uint8_t *adj; // compressed successor lists
  uint64_t nbb; // nb bytes used by adj

  uint8_t f_id; // id of the function used to code the successors lists
  uint8_t k;    // shrinking parameter of the function, if it's the zeta function
} CompAdjListG;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void free_CompAdjListG(CompAdjListG *calg);

CompAdjListG *load_CompAdjListG(char *path, 
  uint8_t f_id, uint8_t k, uint64_t *u);

void write_CompAdjListG(CompAdjListG *calg, char *path);

CompAdjListG *read_CompAdjListG(char *path);

////////////////////////////////////////////////////////////////////////////////////////
// Breadth First Search Algorithm.
////////////////////////////////////////////////////////////////////////////////////////

uint64_t *bfs_CompAdjListG(
  CompAdjListG *calg, uint64_t curr, uint64_t *nbvals);

#endif 
