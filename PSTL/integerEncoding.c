#include "integerEncoding.h"


uint8_t g_k = 2; // the k of zeta_k code ==2 by default must be set with setK for other values

/**
 * Sets the value of g_k to newk (shrinking factor))
 */
void setK(uint8_t newk)
{
  g_k = newk;
}

/**
 * returns the valuie ok g_k
 */
uint8_t getK()
{
  return g_k;
}

/**
 * prints the value of g_k
 */
void printK()
{
  fprintf(stderr, "%u\n", g_k);
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*u", using unary encoding.
 */
uint8_t *unary_encoding(uint8_t *arr, uint64_t *u, uint64_t val)
{
  uint64_t pos = *u;
  set_nth_bit(arr, u, pos + val);
  return arr;
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*u", using gamma encoding.
 */
uint8_t *gamma_encoding(uint8_t *arr, uint64_t *u, uint64_t val)
{
  if (val == 0) {
    unary_encoding(arr, u, 0);
    return arr;
  }
  uint64_t l = 0;
  uint64_t v = val;
  while (v > 0) {
    l++;
    v /= 2;
  }
  unary_encoding(arr, u, l);
  v = val;
  uint64_t pos = * u;
  for (uint64_t i = l - 1; i > 0; i--) {
    write_nth_bit(arr, u, pos + i - 1, v % 2);
    v /= 2;
  }
  return arr;
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*u", using delta encoding.
 */
uint8_t *delta_encoding(uint8_t *arr, uint64_t *u, uint64_t val)
{
  if (val == 0) {
    gamma_encoding(arr, u, 0);
    return arr;
  }
  uint64_t l = 0;
  uint64_t v = val;
  while (v > 0) {
    l++;
    v /= 2;
  }
  gamma_encoding(arr, u, l);
  v = val;
  uint64_t pos = * u;
  for (uint64_t i = l - 1; i > 0; i--) {
    write_nth_bit(arr, u, pos + i - 1, v % 2);
    v /= 2;
  }
  return arr;
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*u", using nibble encoding.
 */
uint8_t *nibble_encoding(uint8_t *arr, uint64_t *u, uint64_t val)
{
  uint64_t l = 0, v1 = 1, v2 = 2,
    nb_z = 0,
    pos = *u;
  uint64_t v = val;
  if (val > 0) {
    while (val < v1 || val > v2 - 1) {
      l++;
      v1 *= 2;
      v2 *= 2;
    }
    l++;
    while ((l + nb_z) % 3 != 0)
      nb_z++;
  } else {
    l = 1;
    nb_z = 2;
  }
  uint64_t cpt = l + nb_z + (l + nb_z) / 3 - 1; //11
  v = val;

  write_nth_bit(arr, u, pos + cpt, v % 2);
  v /= 2;
  cpt--; //10

  write_nth_bit(arr, u, pos + cpt, v % 2);
  v /= 2;
  cpt--; //9

  write_nth_bit(arr, u, pos + cpt, v % 2);
  v /= 2;
  cpt--; //8

  set_nth_bit(arr, u, pos + cpt);

  while (cpt > 0) {
    cpt--;
    if (cpt % 4 == 0) {
      clear_nth_bit(arr, u, pos + cpt);
    } else {
      write_nth_bit(arr, u, pos + cpt, v % 2);
      v /= 2;
    }
  }
  return arr;
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*u", using the minimal binary encoding method
 */
uint8_t *minimal_binary_encode(uint8_t *arr, uint64_t *u, uint64_t x, uint64_t intr)
{
  uint64_t s, z, i, pos, v,
  h = 0, v1 = 1, v2 = 2;
  pos = * u;
  z = intr + 1;

  while (v1 > z || z >= v2) {
    h++;
    v1 *= 2;
    v2 *= 2;
  }
  s = h + 1;

  if (x < u64pow(2, s) - z) {
    v = x;
    for (i = s - 1; i > 0; i--) {
      write_nth_bit(arr, u, pos + i - 1, v % 2);
      v /= 2;
    }
  } else {
    v = x - z + u64pow(2, s);
    for (i = s; i > 0; i--) {
      write_nth_bit(arr, u, pos + i - 1, v % 2);
      v /= 2;
    }
  }
  return arr;
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*u", using the zeta_k encoding.
 */
uint8_t *zeta_k_encoding(uint8_t *arr, uint64_t *u, uint64_t val)
{
  val++;
  uint64_t h = 0, v1 = 1, v2, _2pk = 1, i;
  for (i = 0; i < g_k; i++)
    _2pk *= 2;
  v2 = _2pk;
  while (val < v1 || val > v2 - 1) {
    h++;
    v1 *= _2pk;
    v2 *= _2pk;
  }
  unary_encoding(arr, u, h);
  minimal_binary_encode(arr, u, val - v1, v2 - v1 - 1);
  return arr;
}


/**
 * Decodes a value, from the array "arr", 
 * starting at the bit "*s", and returning the decoed values.
 */
uint64_t unary_decoding(uint8_t *arr, uint64_t *s)
{
  uint64_t cpt = 0;
  for (uint64_t i = * s; 1; i++) {
    if (read_nth_bit(arr, i) == 0) {
      cpt++;
    } else {
      * s = * s + cpt + 1;
      break;
    }
  }
  return cpt;
}

/**
 * Decodes a value, from the array "arr", 
 * starting at the bit "*s", and returning the decoed values.
 */
uint64_t gamma_decoding(uint8_t *arr, uint64_t *s)
{
  uint64_t cpt = * s,
    p = 0, puiss = 1,
    ind, l,
    val = 0;
  l = unary_decoding(arr, & cpt);
  if (l == 0) {
    * s = cpt;
    return 0;
  }
  ind = cpt + l - 2;
  for (p = 0; p < l; p++) {
    val += read_nth_bit(arr, ind - p) * puiss;
    puiss *= 2;
  }
  * s = ind + 1;
  return val;
}

/**
 * Decodes a value, from the array "arr", 
 * starting at the bit "*s", and returning the decoed values.
 */
uint64_t delta_decoding(uint8_t *arr, uint64_t *s)
{
  uint64_t cpt = *s,
    p = 0, puiss = 1,
    ind, l,
    val = 0;
  l = gamma_decoding(arr, & cpt);
  if (l == 0) {
    *s = cpt;
    return 0;
  }
  ind = cpt + l - 2;
  for (p = 0; p < l - 1; p++) {
    val += read_nth_bit(arr, ind - p) * puiss;
    puiss *= 2;
  }
  val += 1 * puiss;
  *s = ind + 1;
  return val;
}

/**
 * Decodes a value, from the array "arr", 
 * starting at the bit "*s", and returning the decoed values.
 */
uint64_t nibble_decoding(uint8_t *arr, uint64_t *s)
{
  uint64_t cpt = *s,
    puiss = 1,
    ind,
    val = 0,
    cptbis = 0;
  while (read_nth_bit(arr, cpt) != 1) {
    cpt += 4;
  }
  cpt += 3;
  for (ind = cpt; ind > *s; ind--) {
    cptbis++;
    if (cptbis % 4 == 0) {
      continue;
    } else {
      val += read_nth_bit(arr, ind) * puiss;
      puiss *= 2;
    }
  }
  *s = cpt + 1;
  return val;
}

/**
 * Decodes a value, from the array "arr", 
 * starting at the bit "*s", and returning the decoed values.
 */
uint64_t zeta_k_decoding(uint8_t *arr, uint64_t *s)
{
  uint64_t val = 0, pos, p = 1, i, left, res,
    h = unary_decoding(arr, s);
  pos = *s;
  left = 1 << h * g_k;

  for (i = h * g_k + g_k - 1; i > 0; i--) {
    if (read_nth_bit(arr, pos + i - 1))
      val += p;
    p *= 2;
  }
  if (val < left) {
    *s = pos + h * g_k + g_k - 1;
    res = val + left;
  } else {
    *s = pos + h * g_k + g_k;
    res = (val * 2) + read_nth_bit(arr, pos + h * g_k + g_k - 1);
  }
  return res - 1;
}


/**
 * Encodes an array of values
 */
uint8_t *array_encoding( uint64_t n,
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val),
  uint64_t *u,  uint64_t *arr)
{
  uint64_t i;
  uint8_t *res = calloc(n * 64, sizeof(uint8_t));
  for (i = 0; i < n; i++)
    encoding_function(res, u, arr[i]);
  adjust_allocation(res, u);
  return res;
}

/**
 * Decodes an array of values 
 */
uint64_t *array_decoding(uint8_t *arr,
  uint64_t decoding_function(uint8_t *arr, uint64_t *s),
  uint64_t n)
{
  uint64_t *res = malloc(n * sizeof(uint64_t));
  uint64_t s = 0;
  for (uint64_t i = 0; i < n; i++)
    res[i] = decoding_function(arr, & s);
  return res;
}

uint64_t unary_expl(uint64_t val)
{
  return val + 1;
}

/**
 * returns the number of bits that val will take if it's coded using the gamma code
 */
uint64_t gamma_expl(uint64_t val)
{
  if (val == 0)
    return 1;
  uint64_t h = 0, v1 = 1, v2 = 2;
  while (val < v1 || val > v2 - 1) {
    h++;
    v1 *= 2;
    v2 *= 2;
  }
  return 2 * h + 2; 
}

/**
 * returns the number of bits that val will take if it's coded using the delta code
 */
uint64_t delta_expl(uint64_t val)
{
  if (val == 0)
    return 1;

  uint64_t h = 0, hbis = 0, v1 = 1, v2 = 2;
  while (val < v1 || val >= v2) {
    h++;
    v1 *= 2;
    v2 *= 2;
  }
  v1 = 1, v2 = 2;
  while (h + 1 < v1 || h + 1 >= v2) {
    hbis++;
    v1 *= 2;
    v2 *= 2;
  }
  return h + 2 * (hbis) + 2;
}

/**
 * returns the number of bits that val will take if it's coded using the nibble code
 */
uint64_t nibble_expl(uint64_t val)
{
  uint64_t h = 0, v1 = 1, v2 = 8;
  if (val == 1 || val == 0)
    return 4;
  while (val < v1 || val >= v2) {
    h++;
    v1 *= 8;
    v2 *= 8;
  }
  return 4 * (h + 1);
}

/**
 * returns the number of bits that val will take if it's coded using the zeta_k code
 */
uint64_t zeta_k_expl(uint64_t val) {
  val++;

  uint64_t h = 0, v1 = 1, v2 = 2, v3 = 2, v4, i, _2pk = 1;
  for (i = 0; i < g_k; i++)
    _2pk *= 2;
  v4 = _2pk;
  while (1) {
    if (v1 <= val && val <= v2 - 1)
      return (h + 1) * (g_k + 1) - 1;
    if (v3 <= val && val <= v4 - 1)
      return (h + 1) * (g_k + 1);
    h++;
    v1 *= _2pk;
    v2 *= _2pk;
    v3 *= _2pk;
    v4 *= _2pk;
  }
}


/**
 * An array containing the encoding functions
 */
uint8_t *( * encoding_functions[5])(uint8_t *arr, uint64_t *u, uint64_t val) = {
  unary_encoding, gamma_encoding, delta_encoding,
  nibble_encoding, zeta_k_encoding
};

/**
 * An array containing the decoding functions
 */
uint64_t( * decoding_functions[5])(uint8_t *arr, uint64_t *s) = {
  unary_decoding, gamma_decoding, delta_decoding,
  nibble_decoding, zeta_k_decoding
};

/**
 * An array containing the expl functions
 */
uint64_t( * expl_functions[5])(uint64_t val) = {
  unary_expl, gamma_expl, delta_expl,
  nibble_expl, zeta_k_expl
};

/**
 * retuns the right encoding function ghosen by id
 */
uint8_t *( * get_encoding_function(uint8_t id))(uint8_t *arr, uint64_t *u, uint64_t val) {
  return encoding_functions[id];
}

/**
 * retuns the right decoding function chosen by id
 */
uint64_t( * get_decoding_function(uint8_t id))(uint8_t *arr, uint64_t *s) {
  return decoding_functions[id];
}

/**
 * retuns the right expl function chosen by id
 */
uint64_t( * get_expl_function(uint8_t id))(uint64_t val) {
  return expl_functions[id];
}