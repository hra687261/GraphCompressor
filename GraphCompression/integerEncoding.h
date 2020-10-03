#ifndef INTEGER_ENCODING
#define INTEGER_ENCODING

#include "tools.h"
#include <stdarg.h> 

/**
 * Number of compression codes
 */
#define NB_CODES 5

/**
 * Ids of the compression codes
 */
#define UNARY_FUNC_ID 0
#define GAMMA_FUNC_ID 1
#define DELTA_FUNC_ID 2
#define NIBBLE_FUNC_ID 3
#define ZETA_K_FUNC_ID 4

////////////////////////////////////////////////////////////////////////////////////////
// functions that encode a value val into an array of bytes starting from the nth bit.
////////////////////////////////////////////////////////////////////////////////////////

void unary_encoding(uint8_t *arr, uint64_t *n, uint64_t val,...);

void gamma_encoding(uint8_t *arr, uint64_t *n, uint64_t val,...);

void delta_encoding(uint8_t *arr, uint64_t *n, uint64_t val,...);

void nibble_encoding(uint8_t *arr, uint64_t *n, uint64_t val,...);

void zeta_k_encoding(uint8_t *arr, uint64_t *n, uint64_t val,...);

////////////////////////////////////////////////////////////////////////////////////////
// functions that decode a value val from an array of bytes starting from the sth bit.
////////////////////////////////////////////////////////////////////////////////////////

uint64_t unary_decoding(uint8_t *arr,uint64_t *s,...);

uint64_t gamma_decoding(uint8_t *arr,uint64_t *s,...);

uint64_t delta_decoding(uint8_t *arr,uint64_t *s,...);

uint64_t nibble_decoding(uint8_t *arr,uint64_t *s,...);

uint64_t zeta_k_decoding(uint8_t *arr,uint64_t *s,...);

////////////////////////////////////////////////////////////////////////////////////////
// functions that give the number of bits that will be used to encode a value val.
////////////////////////////////////////////////////////////////////////////////////////

uint64_t unary_expl(uint64_t val,...);

uint64_t gamma_expl(uint64_t val,...);

uint64_t delta_expl(uint64_t val,...);

uint64_t nibble_expl(uint64_t val,...);

uint64_t zeta_k_expl(uint64_t val,...);

////////////////////////////////////////////////////////////////////////////////////////
//  functions that return an encoding, a decoding or an expected length function.
////////////////////////////////////////////////////////////////////////////////////////

void (*get_encoding_function(uint8_t id))(uint8_t *arr, uint64_t *u, uint64_t val,...);

uint64_t (*get_decoding_function(uint8_t id))(uint8_t *arr, uint64_t *s,...);

uint64_t (*get_expl_function(uint8_t id))(uint64_t val,...);

#endif
