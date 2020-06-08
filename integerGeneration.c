#include "integerGeneration.h"

uint8_t g_wasSeeded = 0;
MTRand g_r;

/**
 * Generates an array of ints following a uniform distribution 
 */
uint64_t *uniform_dist_ints(uint64_t nb_vals, uint64_t upper_bound)
{
  uint64_t *arr = malloc(nb_vals * sizeof(uint64_t));
  if (!g_wasSeeded) {
    g_r = seedRand(1337); //
    g_wasSeeded = 1;
  }
  MTRand r = g_r;
  for (uint64_t i = 0; i < nb_vals; i++)
    arr[i] = 1 + genRandLong( & r) % (upper_bound - 1);
  return arr;
}

//Zipf  https://www.csee.usf.edu/~kchriste/tools/genzipf.c
/**
 * Generates an array of ints following a power distribution 
 */
uint64_t *power_dist_ints(uint64_t nb_vals, double alpha, uint64_t n)
{
  static double c = 0; // Normalization constant
  double z, sum_prob, zipf_value = -1;
  uint64_t i, j;
  if (!g_wasSeeded) {
    g_r = seedRand(1337); //
    g_wasSeeded = 1;
  }
  MTRand r = g_r;
  uint64_t *res = malloc(sizeof(uint64_t) * nb_vals);

  // Compute normalization constant on first call only
  for (i = 1; i <= n; i++)
    c = c + (1.0 / pow((double) i, alpha));
  c = 1.0 / c;

  for (j = 0; j < nb_vals; j++) {
    do {
      z = genRand( & r);
    } while ((z == 0) || (z == 1));

    // Map z to the value
    sum_prob = 0;
    for (i = 1; i <= n; i++) {
      sum_prob = sum_prob + c / pow((double) i, alpha);
      if (sum_prob >= z) {
        zipf_value = i;
        break;
      }
    }

    // Assert that zipf_value is between 1 and N
    if ((zipf_value >= 1) && (zipf_value <= n))
      res[j] = (zipf_value);
    else
      i--;
  }
  return res;
}

/*
algorithm poisson random number (Knuth):
    init:
        Let L ← e−λ, k ← 0 and p ← 1.
    do:
        k ← k + 1.
        Generate uniform random number u in [0,1] and let p ← p × u.
    while p > L.
    return k − 1.
*/

//wikipedia https://en.wikipedia.org/wiki/Poisson_distribution#Generating_Poisson-distributed_random_variables
/**
 * Generates an array of ints following a poisson distribution 
 */
uint64_t *poisson_dist_ints(uint64_t nb_vals, double lambda)
{
  double p, l, u;
  uint64_t *res = malloc(sizeof(uint64_t) * nb_vals);
  uint64_t i, k;
  if (!g_wasSeeded) {
    g_r = seedRand(1337); //
    g_wasSeeded = 1;
  }
  MTRand r = g_r;

  for (i = 0; i < nb_vals; i++) {
    l = pow(EXP, -lambda);
    k = 0;
    p = 1.;
    do {
      k++;
      u = genRand( & r);
      p = p * u;
    } while (p > l);
    res[i] = k - 1;
  }
  return res;
}

/**
 * Generates an array of ints following a binomial distribution 
 */
uint64_t *binomial_dist_ints(uint64_t nb_vals, uint64_t n, double p) {
  uint64_t i, j, val;
  uint64_t *res = malloc(sizeof(uint64_t) * nb_vals);
  if (!g_wasSeeded) {
    g_r = seedRand(1337); //
    g_wasSeeded = 1;
  }
  MTRand r = g_r;

  for (j = 0; j < nb_vals; j++) {
    val = 0;
    for (i = 0; i < n; i++)
      if (genRand( & r) < p) val++;
    res[j] = val;
  }
  return res;
}

//https://github.com/ESultanik/mtwister

/** set initial seeds to mt[STATE_VECTOR_LENGTH] using the generator
 * from Line 25 of Table 1 in: Donald Knuth, "The Art of Computer
 * Programming," Vol. 2 (2nd Ed.) pp.102.
 */
void m_seedRand(MTRand *rand, uint64_t seed)
{
  rand->mt[0] = seed & 0xffffffff;
  for (rand->index = 1; rand->index < STATE_VECTOR_LENGTH; rand->index++) {
    rand->mt[rand->index] = (6069 * rand->mt[rand->index - 1]) & 0xffffffff;
  }
}

/**
 * Creates a new random number generator from a given seed.
 */
MTRand seedRand(uint64_t seed)
{
  MTRand rand;
  m_seedRand( &rand, seed);
  return rand;
}

/**
 * Generates a pseudo-randomly generated long.
 */
uint64_t genRandLong(MTRand * rand)
{
  uint64_t y, kk;
  static uint64_t mag[2] = {
    0x0,
    0x9908b0df
  }; /* mag[x] = x * 0x9908b0df for x = 0,1 */
  if (rand->index >= STATE_VECTOR_LENGTH || rand->index < 0) {
    /* generate STATE_VECTOR_LENGTH words at a time */
    if (rand->index >= STATE_VECTOR_LENGTH + 1 || rand->index < 0) {
      m_seedRand(rand, 4357);
    }
    for (kk = 0; kk < STATE_VECTOR_LENGTH - STATE_VECTOR_M; kk++) {
      y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
      rand->mt[kk] = rand->mt[kk + STATE_VECTOR_M] ^ (y >> 1) ^ mag[y & 0x1];
    }
    for (; kk < STATE_VECTOR_LENGTH - 1; kk++) {
      y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
      rand->mt[kk] = rand->mt[kk + (STATE_VECTOR_M - STATE_VECTOR_LENGTH)] ^ (y >> 1) ^ mag[y & 0x1];
    }
    y = (rand->mt[STATE_VECTOR_LENGTH - 1] & UPPER_MASK) | (rand->mt[0] & LOWER_MASK);
    rand->mt[STATE_VECTOR_LENGTH - 1] = rand->mt[STATE_VECTOR_M - 1] ^ (y >> 1) ^ mag[y & 0x1];
    rand->index = 0;
  }
  y = rand->mt[rand->index++];
  y ^= (y >> 11);
  y ^= (y << 7) & TEMPERING_MASK_B;
  y ^= (y << 15) & TEMPERING_MASK_C;
  y ^= (y >> 18);
  return y;
}

/**
 * Generates a pseudo-randomly generated double in the range [0..1].
 */
double genRand(MTRand *rand)
{
  return ((double) genRandLong(rand) / (uint64_t) 0xffffffff);
}