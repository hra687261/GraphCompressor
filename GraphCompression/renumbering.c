#include "tools.h"

/**
 * Renumbers an edgelist in a file located at "path".
 * The file must contain a pair of integers on every line separated by a space,
 * in which every integer represents a node, and every line is a link going from the node on the left
 * to the node on the right, and the file needs to be sorted by the first and second column.
 * sv is the starting value of the renumbering (the minimum value in the file becomes sv).
 */
void renumbering(char *path, uint8_t sv)
{
  uint64_t
  v1, v2, i, n = 0;

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


int main(int argc, char **argv)
{
  if(argc != 2){
    fprintf(stderr,"Expected One argument, the path to the file of the edge list \n"
    "(pair of positive numbers on everyline, separated by a space, and sorted by the first and second column)\n");
    return EXIT_FAILURE;
  }
  renumbering(argv[1],0);
  return EXIT_SUCCESS;
}