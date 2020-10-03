#ifndef ADJLISTCP
#define ADJLISTCP

#include "integerEncoding.h"

/**
 * Number of sections on the compressed successor lists
 */
#define NB_SECTIONS_CL 3

/**
 * Upper bound of the number of bits per link of the resulting compressed graph
 */
#define UB_BITS_PER_LINK_CL 32

/**
 * CompCodes stores the information on the compression codes used to encode
 * the different sections of the compressed lists of succesors.
 * f_x : is the id of the compression code used for the section x
 * if f_x == 4 aka the zeta code is used to compress the section x
 * then x_k, is the shrinking parameter for that code, 
 * otherwise it's value is irrelevant
 * ** The copy lists are coded using bits **
 */
typedef union _CompCodes_CL {
  struct {
    uint8_t f_ref;    uint8_t ref_k;  // references
    uint8_t f_bcnt;   uint8_t bcnt_k; // block counts
    uint8_t f_res;    uint8_t res_k;  // residuals
  } ccscl_struct;
  uint8_t ccscl_array[NB_SECTIONS_CL*2];
} CompCodes_CL;

typedef struct _CompAdjListCL{
	uint64_t n;   // number of nodes
	uint64_t e;   // number of edges
 
  uint64_t *cd; // n's neighbours are in [ cd[n] .. cd[n+1] [
  uint8_t *adj; // compressed successor lists
  uint64_t nbb; // nb bytes used by adj
  
  CompCodes_CL *ccscl;
} CompAdjListCL;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void free_CompAdjListCL(CompAdjListCL *calcl);

CompAdjListCL *load_CompAdjListCL(char *path,
  CompCodes_CL *ccscl, uint64_t *u,
  uint8_t ws, uint8_t mrc);

void write_CompAdjListCL(CompAdjListCL *calcl, char *path);

CompAdjListCL *read_CompAdjListCL(char *path);

////////////////////////////////////////////////////////////////////////////////////////
// Breadth First Search Algorithm.
////////////////////////////////////////////////////////////////////////////////////////

void decode_CompAdjListCL(CompAdjListCL *calcl, uint64_t node, 
  uint64_t *dest, uint64_t *dlen, uint8_t isfc,
  uint64_t (**decoding_functions)(uint8_t *arr, uint64_t *s, ...));

uint64_t *bfs_CompAdjListCL(
  CompAdjListCL *calcl, uint64_t curr, uint64_t *nbvals);

#endif 
