#ifndef BIT_ARRAY
#define BIT_ARRAY

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h> 
#include <math.h>
#include <assert.h> 
#include <string.h>
#include "tools.h"


#define NB_BYTES_INCR 1000000 // == 8000000 bits

#define CHECK_BIT(B,N) (1 & (B >> N))
#define SET_BIT(B,N) {B |= 1 << N;}
#define CLR_BIT(B,N) {B &= ~(1<<N);}

uint8_t *make_byte_array();

uint8_t *make_byte_array_z();

uint8_t *adjust_allocation(uint8_t *arr, uint64_t *u);

uint8_t *incr_allocation(uint8_t *arr, uint64_t *a);

uint8_t *incr_allocation_z(uint8_t *arr, uint64_t *a);

uint8_t read_nth_bit(uint8_t *arr, uint64_t n);

void set_nth_bit(uint8_t *arr, uint64_t *u, uint64_t n);

void clear_nth_bit(uint8_t *arr, uint64_t *u, uint64_t n);

void write_nth_bit(uint8_t *arr, uint64_t *u, uint64_t n, uint8_t val);

void print_byte_array(uint8_t *arr, uint64_t *u);

#endif









