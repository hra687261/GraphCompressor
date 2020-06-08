#include "tools.h"


/**
 * Returns x raised to the power of n
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

uint64_t max3(uint64_t a, uint64_t b, uint64_t c)
{
  a = (a > b) ? a : b;
  return (a > c) ? a : c;
}

uint64_t func(uint64_t v1, uint64_t v2)
{
  if (v2 >= v1)
    return 2 * (v2 - v1);
  else
    return 2 * (v1 - v2) - 1;
}

uint64_t inv_func(uint64_t val, uint64_t x)
{
  if (x % 2 == 0)
    return val + x / 2;
  else
    return val - (x + 1) / 2;
}

// tail -n +5 
// sort -n -k1,1  -k2,2 -o
// sed -i -e 's/\t/ /g'

void renumbering(char *path, uint8_t sv)
{
  uint64_t
  v1, v2, i, n = 0;

  // "line" will be used to ignore the (ign) first lines of the file containing the edgelist
  FILE *file = fopen(path, "r+");
  while (fscanf(file, "%lu %lu", &v1, &v2) == 2)
    n = max3(n, v1, v2);

  uint64_t *nnn = calloc(n + 1, sizeof(uint64_t));

  rewind(file);
  while (fscanf(file, "%lu %lu", &v1, &v2) == 2) {
    nnn[v1] = 1;
    nnn[v2] = 1;
  }

  uint64_t cpt = 0;
  for (i = sv; i <= n; i++) {
    if (nnn[i] == 0)
      cpt++;
    nnn[i] = cpt;
  }

  rewind(file);
  FILE *wfile = fopen(path, "r+");
  fpos_t pos;

  uint8_t l1, l2;
  uint64_t tmpv1, tmpv2;

  fprintf(stderr, "%lu\n", n);
  while (1) {
    fgetpos(file, & pos);

    if (fscanf(file, "%lu %lu\n", &v1, &v2) != 2)
      break;

    tmpv1 = v1;
    tmpv2 = v2;
    l1 = 0;
    l2 = 0;
    for (; tmpv1 > 0; tmpv1 /= 10)
      l1++;
    for (; tmpv2 > 0; tmpv2 /= 10)
      l2++;
    if (l1 == 0) l1 = 1;
    if (l2 == 0) l2 = 1;
    if (nnn[v1] > 0 || nnn[v2] > 0) {
      tmpv1 = v1 - nnn[v1];
      tmpv2 = v2 - nnn[v2];

      fsetpos(wfile, &pos);
      fprintf(wfile, "%.*lu %.*lu", l1, v1 - nnn[v1], l2, v2 - nnn[v2]);
    }
  }
  fclose(file);
  fclose(wfile);
  free(nnn);
}