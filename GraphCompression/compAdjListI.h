#ifndef ADJLISTI
#define ADJLISTI

#include "integerEncoding.h"

/**
 * Number of sections on the compressed successor lists.
 */
#define NB_SECTIONS_I 6

/**
 * Upper bound of the number of bits per link of the resulting compressed graph.
 */
#define UB_BITS_PER_LINK_I 32

typedef union _CompCodes_I {
  struct {
    uint8_t f_ref;    uint8_t ref_k;  // references
    uint8_t f_bcnt;   uint8_t bcnt_k; // block counts
    uint8_t f_cb;     uint8_t cb_k;   // copy blocks
    uint8_t f_int;    uint8_t int_k;  // intervals
    uint8_t f_res;    uint8_t res_k;  // residuals
  } ccsi_struct;
  uint8_t ccsi_array[NB_SECTIONS_I*2];
} CompCodes_I;

typedef struct _CompAdjListI{
  uint64_t n;  // number of nodes
  uint64_t e;  // number of edges
  uint64_t md; // biggest number of successors of any node

  uint64_t *cd; // n's neighbours are in [ cd[n] .. cd[n+1] [
  uint8_t *adj; // compressed successor lists
  uint64_t nbb; // length of adj

  uint64_t th; // threshold

  CompCodes_I *ccsi;  // an array of size NB_IND*2, in which for n the id of a section
                      // c_ids[n*2] contains the id of the compression code used for coding
                      // the section having n as an id, and c_ids[n*2+1] contains the 
                      // shriking parameter (k) if the used code is zeta
} CompAdjListI;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void free_CompAdjListI(CompAdjListI *cali);

CompAdjListI *load_CompAdjListI(char *path,
  CompCodes_I *ccsi, uint64_t *u,
  uint64_t ws, uint64_t mrc, uint64_t threshold);

void write_CompAdjListI(CompAdjListI *cali, char *path);

CompAdjListI *read_CompAdjListI(char *path);

////////////////////////////////////////////////////////////////////////////////////////
// Breadth First Search Algorithm.
////////////////////////////////////////////////////////////////////////////////////////

void decode_CompAdjListI(CompAdjListI *cali, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t m_nb_intrs,uint64_t *intrvs,
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...));

uint64_t *bfs_CompAdjListI(
  CompAdjListI *cali, uint64_t curr, uint64_t *nbvals);

#endif
