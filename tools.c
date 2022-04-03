#include "tools.h"

/**
 * writes val%2 into nth bit of arr.
 */
void write_nth_bit(uint8_t *arr, uint64_t n, uint8_t val)
{
  if (val % 2 == 1) {
    SET_BIT( *(uint8_t *) &arr[n / 8], n % 8);
  } else
    CLR_BIT( *(uint8_t *) &arr[n / 8], n % 8);
}

/**
 * returns the value of x raised to the power of n.
 */
uint64_t u64pow(uint64_t x, uint64_t n)
{
  uint64_t y = 1;
  while (1) {
    if (n % 2 == 1)
      y *= x;
    n = n / 2;
    if (n == 0)
      break;
    x *= x;
  }
  return y;
}

/**
 * returns the max value between a, b and c.
 */
uint64_t max3(uint64_t a, uint64_t b, uint64_t c)
{
  a = (a > b) ? a : b;
  return (a > c) ? a : c;
}

/**
 * the v map.
 */
uint64_t v_func(uint64_t v1, uint64_t v2)
{
  if (v2 >= v1)
    return 2 * (v2 - v1);
  else
    return 2 * (v1 - v2) - 1;
}

/**
 * inverted v map.
 */
uint64_t inv_v_func(uint64_t val, uint64_t x)
{
  if (x % 2 == 0)
    return val + x / 2;
  else
    return val - (x + 1) / 2;
}
