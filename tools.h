#ifndef TOOLS
#define TOOLS

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#define NB_NODES 1000000 

uint64_t u64pow(uint64_t x,uint64_t n);

uint64_t max3(uint64_t a,uint64_t b,uint64_t c);

uint64_t func(uint64_t v1,uint64_t v2);

uint64_t inv_func(uint64_t val,uint64_t x);

void renumbering(char *path, uint8_t ign, uint8_t sv);

#endif