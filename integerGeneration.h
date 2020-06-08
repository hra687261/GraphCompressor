#ifndef INTEGER_GENERATION
#define INTEGER_GENERATION

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h> 
#include <time.h>

#define EXP 2.718281828459045

uint64_t *uniform_dist_ints(uint64_t nb_vals,uint64_t upper_bound);

uint64_t *power_dist_ints(uint64_t nb_vals,double alpha,uint64_t n);

uint64_t *poisson_dist_ints(uint64_t nb_vals,double lambda);

uint64_t *binomial_dist_ints(uint64_t nb_vals,uint64_t n,double p);



//https://github.com/ESultanik/mtwister

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397 /* changes to STATE_VECTOR_LENGTH also require changes to this */
#define UPPER_MASK		0x80000000
#define LOWER_MASK		0x7fffffff
#define TEMPERING_MASK_B	0x9d2c5680
#define TEMPERING_MASK_C	0xefc60000

typedef struct tagMTRand {
  unsigned long mt[STATE_VECTOR_LENGTH];
  int index;
} MTRand;

MTRand seedRand(uint64_t seed);
uint64_t genRandLong(MTRand* rand);
double genRand(MTRand* rand);

#endif
