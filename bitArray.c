#include "bitArray.h"

/**
 * Creates and allocates an array of bytes big enough for NB_BYTES_INCR*8 bits
 */
uint8_t *make_byte_array()
{
  return malloc(NB_BYTES_INCR * sizeof(uint8_t));
}

/**
 * Creates and allocates an array of bytes big enough for NB_BYTES_INCR*8 bits
 * and initializes the allocated memory to 0
 */
uint8_t *make_byte_array_z()
{
  return calloc(NB_BYTES_INCR, sizeof(uint8_t));
}

/**
 * Changers the allocated memory for a byte array
 * to be exactly big enough to fit "*u" bits
 */
uint8_t *adjust_allocation(uint8_t *arr, uint64_t *u)
{
  uint8_t *tmp = realloc(arr, ( *u / 8 + 1) * sizeof(uint8_t));
  assert(tmp);
  arr = tmp;
  return arr;
}

/**
 * Increases the byte array's allocated memory by NB_BYTES_INCR bytes
 */
uint8_t *incr_allocation(uint8_t *arr, uint64_t *a)
{
  uint8_t *tmp = realloc(arr, ( *a + NB_BYTES_INCR) * sizeof(uint8_t));
  assert(tmp);
  arr = tmp;
  *a += NB_BYTES_INCR;
  return arr;
}

/**
 * Increases the byte array's allocated memory by NB_BYTES_INCR bytes
 * and initializes the new memory to 0
 */
uint8_t *incr_allocation_z(uint8_t *arr, uint64_t *a)
{
  uint8_t *tmp = realloc(arr, ( *a + NB_BYTES_INCR) * sizeof(uint8_t));
  assert(tmp);
  arr = tmp;
  memset(arr + *a, 0, NB_BYTES_INCR * sizeof(uint8_t)); // might fail
  *a += NB_BYTES_INCR;
  return arr;
}

/**
 * Returns the value of the nth bit of the BitArray (0 or 1)
 */
uint8_t read_nth_bit(uint8_t *arr, uint64_t n)
{
  return CHECK_BIT( *(uint8_t *) &arr[n / 8], n % 8);
}

/**
 * Sets the nth bit to 1
 */
void set_nth_bit(uint8_t *arr, uint64_t *u, uint64_t n)
{
  if (n >= *u) *u = n + 1;
  SET_BIT( *(uint8_t *) &arr[n / 8], n % 8);
}

/**
 * Clears the nth bit (sets it to 0)
 */
void clear_nth_bit(uint8_t *arr, uint64_t *u, uint64_t n)
{
  if (n >= *u) *u = n + 1;
  CLR_BIT( *(uint8_t *) &arr[n / 8], n % 8);
}

/**
 * writes the value v%2 to the nth bit of the byte arrat
 */
void write_nth_bit(uint8_t *arr, uint64_t *u, uint64_t n, uint8_t v)
{
  if (n >= *u) *u = n + 1;
  if (v % 2 == 1) {
    SET_BIT( *(uint8_t *) &arr[n / 8], n % 8);
  } else if (v % 2 == 0)
    CLR_BIT( *(uint8_t *) &arr[n / 8], n % 8);
}

/**
 * Prints the content of the BitArray (all the bits from 0 to *u-1 included)
 */
void print_byte_array(uint8_t *arr, uint64_t *u)
{
  for (uint64_t i = 0; i < *u; i++)
    printf("%u", read_nth_bit(arr, i));
  printf("\n");
}