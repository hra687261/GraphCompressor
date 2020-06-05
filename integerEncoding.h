#ifndef INTEGER_ENCODING
#define INTEGER_ENCODING

#include "bitArray.h"


/**
 * Encodes the value val in unary code
 * writes the coded value in the uint8_t
 */
uint8_t *unary_encoding(uint8_t *arr, uint64_t *u, uint64_t val);

uint8_t *gamma_encoding(uint8_t *arr, uint64_t *u, uint64_t val);

uint8_t *delta_encoding(uint8_t *arr, uint64_t *u, uint64_t val);

uint8_t *nibble_encoding(uint8_t *arr, uint64_t *u, uint64_t val);

uint8_t *zeta_k_encoding(uint8_t *arr, uint64_t *u, uint64_t val);


uint64_t unary_decoding(uint8_t *arr,uint64_t *s);

uint64_t gamma_decoding(uint8_t *arr,uint64_t *s);

uint64_t delta_decoding(uint8_t *arr,uint64_t *s);

uint64_t nibble_decoding(uint8_t *arr,uint64_t *s);

uint64_t zeta_k_decoding(uint8_t *arr,uint64_t *s);


uint8_t *array_encoding(
    uint64_t nb_vals, 
    uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val), 
    uint64_t *u, 
    uint64_t *arr);

uint64_t *array_decoding(
    uint8_t *arr, 
    uint64_t decoding_function(uint8_t *arr,uint64_t *s), 
    uint64_t nb_vals);

uint64_t unary_expl(uint64_t val);

uint64_t gamma_expl(uint64_t val);

uint64_t delta_expl(uint64_t val);

uint64_t nibble_expl(uint64_t val);

uint64_t zeta_k_expl(uint64_t val);

void setK(uint8_t k);

uint8_t getK();

void printK();


uint8_t *(*get_encoding_function(uint8_t id))(uint8_t *arr, uint64_t *u, uint64_t val);

uint64_t (*get_decoding_function(uint8_t id))(uint8_t *arr, uint64_t *s);

uint64_t (*get_expl_function(uint8_t id))(uint64_t val);


#endif
