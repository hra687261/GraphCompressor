#ifndef ADJLISTIV2
#define ADJLISTIV2

//9 62409146
#include "integerEncoding.h"

typedef struct _CompAdjList{
	uint64_t n; // number of nodes
	uint64_t e; // number of edges
  uint64_t md; // max number of successors of any node
  
  uint64_t *cd; // n's neighbours are in [ cd[n] .. cd[n+1] 
  uint8_t *adj;
  uint64_t nbb; //length of adj
  uint64_t cds; //length of cd
 
  uint64_t ws; // window size
  uint64_t j; // size of the "jump"
  uint64_t th; // threshold
}CompAdjList;

void free_CompAdjList(CompAdjList *alg);

CompAdjList *load_CompAdjList(char *path, uint8_t ign, 
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val), 
  uint64_t expl_function(uint64_t val), uint64_t *u, 
  uint64_t j, uint64_t ws, 
  uint64_t mrc, uint64_t threshold);

void write_CompAdjList(CompAdjList *alg, uint8_t id, char *path);

CompAdjList *read_CompAdjList(char *path, uint8_t *id);

uint64_t get_node_addrs(
  uint64_t node, CompAdjList *alg,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s));

void decode_CompAdjList(CompAdjList *alg, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s),
  uint64_t m_nb_intrs,uint64_t *intrvs);
  
uint64_t *bfs_CompAdjList(
  CompAdjList *alg,uint64_t curr, uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s));

void decode_CompAdjList_bis(CompAdjList *alg, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s),
  uint64_t m_nb_intrs,uint64_t *intrvs,
  uint64_t **hist, uint64_t *lens);

uint64_t *bfs_CompAdjList_bis(
  CompAdjList *alg,uint64_t curr, uint64_t *nbvals,
  uint64_t decoding_function(uint8_t *arr,uint64_t *s));

uint8_t *mk_has_successors(
  CompAdjList *al,uint64_t decoding_function(uint8_t *arr, uint64_t *s));

void decode_with_window(
  CompAdjList *alg, uint64_t node,
  uint64_t *window_succ,
  uint64_t *window_nodes,
  uint64_t *window_lens,
  uint64_t *ldecp,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s));
  
void normalize (CompAdjList *al, uint8_t *has_successors, double *pr);

double *power_iteration(
  CompAdjList *al, uint64_t nb_it, double alpha,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s));

#endif