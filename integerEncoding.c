#include "integerEncoding.h"

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*n", using unary encoding.
 */
void unary_encoding(uint8_t *arr, uint64_t *n, uint64_t val, ...)
{
  uint64_t pos = *n;
  SET_BIT(*(uint8_t *) &arr[(pos + val) / 8], (pos + val) % 8);
  *n = pos + val + 1;
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*n", using gamma encoding.
 */
void gamma_encoding(uint8_t *arr, uint64_t *n, uint64_t val, ...)
{
  if (val == 0) {
    unary_encoding(arr, n, 0);
    return;
  }
  uint64_t l = 0;
  uint64_t v = val;
  while (v > 0) {
    l++;
    v /= 2;
  }
  unary_encoding(arr, n, l);
  v = val;
  for (uint64_t i = l - 1; i > 0; i--) {
    write_nth_bit(arr, *n + i - 1, v % 2);
    v /= 2;
  }
  *n += l - 1;
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*n", using delta encoding.
 */
void delta_encoding(uint8_t *arr, uint64_t *n, uint64_t val, ...)
{
  if (val == 0) {
    gamma_encoding(arr, n, 0);
    return;
  }
  uint64_t l = 0;
  uint64_t v = val;
  while (v > 0) {
    l++;
    v /= 2;
  }
  gamma_encoding(arr, n, l);
  v = val;
  
  for (uint64_t i = l - 1; i > 0; i--) {
    write_nth_bit(arr, *n + i - 1, v % 2);
    v /= 2;
  }
  *n += l - 1;
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*n", using nibble encoding.
 */
void nibble_encoding(uint8_t *arr, uint64_t *n, uint64_t val, ...)
{
  uint64_t l = 0, v1 = 1, v2 = 2,
    nb_z = 0,
    pos = *n;
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
  uint64_t cpt = l + nb_z + (l + nb_z) / 3 - 1; 
  v = val;

  write_nth_bit(arr, pos + cpt, v % 2);
  *n = pos + cpt + 1;
  v /= 2;
  cpt--; 

  write_nth_bit(arr, pos + cpt, v % 2);
  v /= 2;
  cpt--; 

  write_nth_bit(arr, pos + cpt, v % 2);
  v /= 2;
  cpt--; 

  SET_BIT(*(uint8_t *) &arr[(pos + cpt) / 8], (pos + cpt) % 8);

  while (cpt > 0) {
    cpt--;
    if (cpt % 4 == 0) {
      CLR_BIT(*(uint8_t *) &arr[(pos + cpt) / 8], (pos + cpt) % 8);
    } else {
      write_nth_bit(arr, pos + cpt, v % 2);
      v /= 2;
    }
  }
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*n", using the minimal binary encoding method
 */
void minimal_binary_encode(uint8_t *arr, uint64_t *n, uint64_t x, uint64_t intr)
{
  uint64_t s, z, i, pos, v,
  h = 0, v1 = 1, v2 = 2;
  pos = *n;
  z = intr + 1;

  while (v1 > z || z >= v2) {
    h++;
    v1 *= 2;
    v2 *= 2;
  }
  s = h + 1;

  if (x < u64pow(2, s) - z) {
    v = x;
    *n = pos + s - 1;
    for (i = s - 1; i > 0; i--) {
      write_nth_bit(arr, pos + i - 1, v % 2);
      v /= 2;
    }
  } else {
    v = x - z + u64pow(2, s);
    *n = pos + s;
    for (i = s; i > 0; i--) {
      write_nth_bit(arr, pos + i - 1, v % 2);
      v /= 2;
    }
  }
}

/**
 * Encodes the value "val", into the array "arr", 
 * starting at the bit "*u", using the zeta_k encoding.
 */
void zeta_k_encoding(uint8_t *arr, uint64_t *n, uint64_t val, ...)
{
  val++;
    
  va_list a_list;
  va_start(a_list, val);
  uint8_t k = (uint8_t) va_arg(a_list, uint32_t);
  uint64_t h = 0, v1 = 1, v2, _2pk = 1, i;
  for (i = 0; i < k; i++)
    _2pk *= 2;
  v2 = _2pk;
  while (val < v1 || val > v2 - 1) {
    h++;
    v1 *= _2pk;
    v2 *= _2pk;
  }
  unary_encoding(arr, n, h);
  minimal_binary_encode(arr, n, val - v1, v2 - v1 - 1);
}

/**
 * Decodes a value, from the array "arr", 
 * starting at the bit "*s", and returning the decoed values.
 */
uint64_t unary_decoding(uint8_t *arr, uint64_t *s, ...)
{
  uint64_t res = 0, i = *s;
  while(CHECK_BIT(*(uint8_t *) &arr[i / 8], i % 8) == 0){
    i++;
  }
  res = i - *s;
  *s = i + 1;
  return res;
}

/**
 * Decodes a value, from the array "arr", 
 * starting at the bit "*s", and returning the decoed values.
 */
uint64_t gamma_decoding(uint8_t *arr, uint64_t *s, ...)
{
  uint64_t cpt = * s,
    p = 0, puiss = 1,
    ind, l,
    val = 0;
  l = unary_decoding(arr, &cpt);
  if (l == 0) {
    * s = cpt;
    return 0;
  }
  ind = cpt + l - 2;
  for (p = 0; p < l; p++) {
    val += CHECK_BIT( *(uint8_t *) &arr[(ind - p) / 8], (ind - p) % 8) * puiss; 
    puiss *= 2;
  }
  *s = ind + 1;
  return val;
}

/**
 * Decodes a value, from the array "arr", 
 * starting at the bit "*s", and returning the decoed values.
 */
uint64_t delta_decoding(uint8_t *arr, uint64_t *s, ...)
{
  uint64_t cpt = *s,
    p = 0, puiss = 1,
    ind, l,
    val = 0;
  l = gamma_decoding(arr, &cpt);
  if (l == 0) {
    *s = cpt;
    return 0;
  }
  ind = cpt + l - 2;
  for (p = 0; p < l - 1; p++) {
    val += CHECK_BIT( *(uint8_t *) &arr[(ind - p) / 8], (ind - p) % 8) * puiss;
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
uint64_t nibble_decoding(uint8_t *arr, uint64_t *s, ...)
{
  uint64_t cpt = *s,
    puiss = 1,
    ind,
    val = 0,
    cptbis = 0;
  while (CHECK_BIT( *(uint8_t *) &arr[cpt / 8], cpt % 8) != 1) {
    cpt += 4;
  }
  cpt += 3;
  for (ind = cpt; ind > *s; ind--) {
    cptbis++;
    if (cptbis % 4 == 0) {
      continue;
    } else {
      val += CHECK_BIT( *(uint8_t *) &arr[ind / 8], ind % 8) * puiss;
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
uint64_t zeta_k_decoding(uint8_t *arr, uint64_t *s, ...)
{
  uint64_t val = 0, pos, p = 1, i, left, res,
    h = unary_decoding(arr, s);
  
  va_list a_list;
  va_start(a_list, s);
  uint8_t k = (uint8_t) va_arg(a_list, uint32_t);
  pos = *s;
  left = 1 << h * k;

  for (i = h * k + k - 1; i > 0; i--) {
    if (CHECK_BIT( *(uint8_t *) &arr[(pos + i - 1) / 8], (pos + i - 1) % 8))
      val += p;
    p *= 2;
  }
  if (val < left) {
    *s = pos + h * k + k - 1;
    res = val + left;
  } else {
    *s = pos + h * k + k;
    res = (val * 2) + CHECK_BIT( *(uint8_t *) &arr[(pos + h * k + k - 1) / 8], (pos + h * k + k - 1) % 8);
  }
  return res - 1;
}

uint64_t unary_expl(uint64_t val, ...)
{
  return val + 1;
}

/**
 * returns the number of bits that val will take if it's coded using the gamma code
 */
uint64_t gamma_expl(uint64_t val, ...)
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
uint64_t delta_expl(uint64_t val, ...)
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
uint64_t nibble_expl(uint64_t val, ...)
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
uint64_t zeta_k_expl(uint64_t val, ...) {
  val++;
  
  va_list a_list;
  va_start(a_list,val);
  uint8_t k = (uint8_t) va_arg(a_list, uint32_t);

  uint64_t h = 0, v1 = 1, v2 = 2, v3 = 2, v4, i, _2pk = 1;
  for (i = 0; i < k; i++)
    _2pk *= 2;
  v4 = _2pk;
  while (1) {
    if (v1 <= val && val <= v2 - 1)
      return (h + 1) * (k + 1) - 1;
    if (v3 <= val && val <= v4 - 1)
      return (h + 1) * (k + 1);
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
void (*encoding_functions[NB_CODES])(uint8_t *arr, uint64_t *u, uint64_t val, ...) = {
  unary_encoding, gamma_encoding, delta_encoding, nibble_encoding, zeta_k_encoding
};

/**
 * An array containing the decoding functions
 */
uint64_t (*decoding_functions[NB_CODES])(uint8_t *arr, uint64_t *s, ...) = {
  unary_decoding, gamma_decoding, delta_decoding, nibble_decoding, zeta_k_decoding
};

/**
 * An array containing the expl functions
 */
uint64_t (*expl_functions[NB_CODES])(uint64_t val, ...) = {
  unary_expl, gamma_expl, delta_expl, nibble_expl, zeta_k_expl
};

/**
 * retuns the encoding function corresponding to the function id f_id
 */
void (*get_encoding_function(uint8_t id))(uint8_t *arr, uint64_t *u, uint64_t val, ...) 
{
  return encoding_functions[id];
}

/**
 * retuns the decoding function corresponding to the function id f_id
 */
uint64_t (*get_decoding_function(uint8_t id))(uint8_t *arr, uint64_t *s, ...)
{
  return decoding_functions[id];
}

/**
 * retuns the expl function corresponding to the function id f_id
 */
uint64_t (*get_expl_function(uint8_t id))(uint64_t val, ...)
{
  return expl_functions[id];
}