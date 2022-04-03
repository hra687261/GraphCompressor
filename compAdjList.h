#ifndef ADJLIST
#define ADJLIST

#include "integerEncoding.h"

/**
 * Number of sections on the compressed successor lists
 */
#define NB_SECTIONS 6

/**
 * Upper bound of the number of bits per link of the resulting compressed graph
 */
#define UB_BITS_PER_LINK 32

/**
 * CompCodes stores the information on the compression codes used to encode
 * the different sections of the compressed lists of succesors.
 * f_x : is the id of the compression code used for the section x
 * if f_x == 4 aka the zeta code is used to compress the section x
 * then x_k, is the shrinking parameter for that code, 
 * otherwise it's value is irrelevant
 */
typedef union _CompCodes {
  struct {
    uint8_t f_ub;     uint8_t ub_k;    // number of used bits
    uint8_t f_ref;    uint8_t ref_k;   // references
    uint8_t f_bcnt;   uint8_t bcnt_k;  // block counts
    uint8_t f_cb;     uint8_t cb_k;    // copy blocks
    uint8_t f_int;    uint8_t int_k;   // intervals
    uint8_t f_res;    uint8_t res_k;   // residuals
  } ccs_struct;
  uint8_t ccs_array[NB_SECTIONS*2];   
} CompCodes;


typedef struct _CompAdjList {
  uint64_t n;  // number of nodes
  uint64_t e;  // number of edges
  uint64_t md; // biggest number of successors of any node

  uint64_t *cd; // n's neighbours are the interval of bits [ cd[n] .. cd[n+1] [
  uint8_t *adj; // compressed successor lists
  uint64_t nbb; // length of adj
  uint64_t cds; // length of cd

  uint64_t ws; // window size
  uint64_t j;  // jump size 
  uint64_t th; // threshold value

  CompCodes *ccs; 
} CompAdjList;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void free_CompAdjList(CompAdjList *cal);

CompAdjList *load_CompAdjList(char *path,
  CompCodes *ccs, uint64_t *u, uint64_t jump, 
  uint64_t ws, uint64_t mrc, uint64_t threshold);

void write_CompAdjList(CompAdjList *cal, char *path);

CompAdjList *read_CompAdjList(char *path);

////////////////////////////////////////////////////////////////////////////////////////
// Breadth First Search Algorithm.
////////////////////////////////////////////////////////////////////////////////////////

uint64_t get_node_addrs(uint64_t node, CompAdjList *alg);

void decode_CompAdjList(CompAdjList *cal, uint64_t node,
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t m_nb_intrs, uint64_t *intrvs,
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...));

uint64_t *bfs_CompAdjList( CompAdjList *cal, uint64_t curr, uint64_t *nbvals);

void decode_CompAdjList_bis(
  CompAdjList *cal, uint64_t node, uint64_t *dest, 
  uint64_t *dlen, uint8_t isfc, uint64_t m_nb_intrs,
  uint64_t *intrvs, uint64_t **hist, uint64_t *lens,
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...));

uint64_t *bfs_CompAdjList_bis(
  CompAdjList *cal, uint64_t curr, uint64_t *nbvals);

////////////////////////////////////////////////////////////////////////////////////////
// Pagerank Algorithm.
////////////////////////////////////////////////////////////////////////////////////////

void decode_with_window(
  CompAdjList *cal, uint64_t node,
  uint64_t *window_succ,
  uint64_t *window_nodes,
  uint64_t *window_lens,
  uint64_t *ldecp);

void normalize(CompAdjList *cal, double *pr);

uint8_t *mk_has_successors(CompAdjList *cal);

double *power_iteration(
  CompAdjList *cal, uint64_t nb_it, double alpha);

#endif
