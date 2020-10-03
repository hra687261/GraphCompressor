#ifndef ADJLISTCB
#define ADJLISTCB

#include "integerEncoding.h"

/**
 * Number of sections on the compressed successor lists.
 */
#define NB_SECTIONS_CB 4

/**
 * Upper bound of the number of bits per link of the resulting compressed graph.
 */
#define UB_BITS_PER_LINK_I 32

/**
 * CompCodes_CB stores the information on the compression codes used to encode
 * the different sections of the compressed lists of succesors.
 * f_x : is the id of the compression code used for the section x
 * if f_x == 4 aka the zeta code is used to compress the section x
 * then x_k, is the shrinking parameter for that code, 
 * otherwise it's value is irrelevant.
 */
typedef union _CompCodes_CB {
  struct {
    uint8_t f_ref;    uint8_t ref_k;  // references
    uint8_t f_bcnt;   uint8_t bcnt_k; // block counts
    uint8_t f_cb;     uint8_t cb_k;   // copy blocks
    uint8_t f_res;    uint8_t res_k;  // residuals
  } ccscb_struct;
  uint8_t ccscb_array[NB_SECTIONS_CB*2];
} CompCodes_CB;

typedef struct _CompAdjListCB {
  uint64_t n;   // number of nodes
  uint64_t e;   // number of edges

  uint64_t *cd; // n's neighbours are in [ cd[n] .. cd[n+1] [
  uint8_t *adj; // compressed successor lists
  uint64_t nbb; // nb bytes used by adj

  CompCodes_CB *ccscb;
} CompAdjListCB;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void free_CompAdjListCB(CompAdjListCB *calcb);

CompAdjListCB *load_CompAdjListCB(char *path,
  CompCodes_CB *ccscb, uint64_t *u, 
  uint8_t ws, uint8_t mrc);

void write_CompAdjListCB(CompAdjListCB *calcb, char *path);

CompAdjListCB *read_CompAdjListCB(char *path);

////////////////////////////////////////////////////////////////////////////////////////
// Breadth First Search Algorithm.
////////////////////////////////////////////////////////////////////////////////////////

void decode_CompAdjListCB(CompAdjListCB *calcb, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...));

uint64_t *bfs_CompAdjListCB(
  CompAdjListCB *calcb, uint64_t curr, uint64_t *nbvals);

#endif
