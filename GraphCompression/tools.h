#ifndef TOOLS
#define TOOLS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h> 
#include <time.h> 
#include <string.h>

// returns the value of the Nth bit of a byte B
#define CHECK_BIT(B,N) (1 & (B >> N))

// sets the Nth bit of a byte B to 1
#define SET_BIT(B,N) {B |= 1 << N;}

// sets the Nth bit of a byte B to 0
#define CLR_BIT(B,N) {B &= ~(1<<N);}

void write_nth_bit(uint8_t *arr, uint64_t n, uint8_t val);

uint64_t u64pow(uint64_t x,uint64_t n);

uint64_t max3(uint64_t a,uint64_t b,uint64_t c);

uint64_t v_func(uint64_t v1,uint64_t v2);

uint64_t inv_v_func(uint64_t val,uint64_t x);

#endif