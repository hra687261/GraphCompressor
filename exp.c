#include "exp.h"
#include <string.h>

/**
 * writes n values from the array arr to the file located at path
 */
void idist_to_file(uint64_t *arr, uint64_t n, char *path)
{
  uint64_t i;
  FILE *file = fopen(path, "w");
  for (i = 0; i < n; i++) {
    fprintf(file, "%lu\n", arr[i]);
  }
  fclose(file);
}


/**
 * Plots the graph associated with a file of integers located at file_path,
 * it's supposed to contain one integer by lane and plot their distribution curve
 */
void plot_idfile(char *file_path)
{
  char *command;

  // réecriture du fichier sous forme couples entier : nombre d'occurences
  command = malloc(sizeof(char) * (56 + strlen(file_path) * 2));
  sprintf(command, "echo \"$(sort -n %s | uniq -c | awk '{print $2, $1}')\" > %s", file_path, file_path);
  assert(system(command) != -1);
  free(command);

  //dessination du graphe
  command = malloc(sizeof(char) * (145 + strlen(file_path)));
  sprintf(command, "gnuplot -p -e \"\
        plot '%s' w l \"", file_path);
  assert(system(command) != -1);

  free(command);
}

/**
 * takes and encoding function, a decoding function, a number nb_vals that's the number of values,
 * and array containing a values that will be encoded and a value 'print" saying whether or not
 * to print the result of the compression
 * the function encodes the n values from the array
 * then decodes them and makes sure that the decoded values are equal to the coded ones
 */
void single_test(
  uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val),
  uint64_t decoding_function(uint8_t *arr, uint64_t *s),
  uint64_t nb_vals,
  uint64_t *arr,
  uint8_t print) {
  uint64_t i, *res, u;
  uint8_t *b_arr;

  if (print) {
    for (i = 0; i < nb_vals; i++)
      printf("%lu ", arr[i]);
    printf("\n");
  }

  u = 0;
  printf("nombre de valeurs à coder : %lu\n", nb_vals);
  b_arr = array_encoding(nb_vals, encoding_function, &u, arr);
  res = array_decoding(b_arr, decoding_function, nb_vals);
  printf("nombre de bits utilisés : %lu\n", u);

  for (i = 0; i < nb_vals; i++) {
    if (arr[i] != res[i])
      printf("%lu  %lu\n", arr[i], res[i]);
    assert(arr[i] == res[i]);
  }
  printf("décodage réussi!\n");

  free(res);
  free(b_arr);
}

/**
 * Tests the encoding and decoding functions 
 * on an array of uint64_t of size nb_vals
 * it makes sure that the encodings and decodings are valid
 */
void encoding_decoding_tests(uint64_t nb_vals)
{
  uint64_t *arr; //,*res,i;

  arr = uniform_dist_ints(nb_vals, 1000);

  printf("\nUnary encoding test :\n");
  single_test(unary_encoding, unary_decoding, nb_vals, arr, 0);

  printf("\nGamma encoding test :\n");
  single_test(gamma_encoding, gamma_decoding, nb_vals, arr, 0);

  printf("\nDelta encoding test :\n");
  single_test(delta_encoding, delta_decoding, nb_vals, arr, 0);

  printf("\nNibble encoding test :\n");
  single_test(nibble_encoding, nibble_decoding, nb_vals, arr, 0);

  setK(2);
  printf("\nZeta_2 encoding test :\n");
  single_test(zeta_k_encoding, zeta_k_decoding, nb_vals, arr, 0);

  setK(3);
  printf("\nZeta_3 encoding test :\n");
  single_test(zeta_k_encoding, zeta_k_decoding, nb_vals, arr, 0);

  setK(4);
  printf("\nZeta_4 encoding test :\n");
  single_test(zeta_k_encoding, zeta_k_decoding, nb_vals, arr, 0);

  free(arr);
}

/**
 * tests the unfirom distribution generationg function
 * plots the curve of the distribution of the generated values
 */
void test_uniform(uint64_t nb_vals, char *file_name, uint64_t max)
{
  uint64_t *arr;
  char *file_path;

  file_path = malloc(sizeof(char) * (strlen(file_name) + 5));
  sprintf(file_path, "%s.txt", file_name);
  arr = uniform_dist_ints(nb_vals, max);

  idist_to_file(arr, nb_vals, file_path);
  plot_idfile(file_path);

  free(file_path);
}

/**
 * tests the poisson distribution generationg function
 * plots the curve of the distribution of the generated values
 */
void test_poisson(uint64_t nb_vals, char *file_name, double lambda)
{
  uint64_t *arr;
  char *file_path;

  file_path = malloc(sizeof(char) * (strlen(file_name) + 5));
  sprintf(file_path, "%s.txt", file_name);
  arr = poisson_dist_ints(nb_vals, lambda);

  idist_to_file(arr, nb_vals, file_path);
  plot_idfile(file_path);

  free(arr);
  free(file_path);
}
/**
 * tests the power distribution generationg function
 * plots the curve of the distribution of the generated values
 */
void test_power(uint64_t nb_vals, char *file_name, double alpha, uint64_t n)
{
  uint64_t *arr;
  char *file_path;

  file_path = malloc(sizeof(char) * (strlen(file_name) + 5));
  sprintf(file_path, "%s.txt", file_name);
  arr = power_dist_ints(nb_vals, alpha, n);

  idist_to_file(arr, nb_vals, file_path);
  plot_idfile(file_path);

  free(arr);
  free(file_path);
}

/**
 * tests the binomial distribution generationg function
 * plots the curve of the distribution of the generated values
 */
void test_binomial(uint64_t nb_vals, char *file_name, uint64_t n, double p)
{
  uint64_t *arr;
  char *file_path;

  file_path = malloc(sizeof(char) * (strlen(file_name) + 5));
  sprintf(file_path, "%s.txt", file_name);
  arr = binomial_dist_ints(nb_vals, n, p);

  idist_to_file(arr, nb_vals, file_path);
  plot_idfile(file_path);

  free(arr);
  free(file_path);
}

/**
 * compares the number of used bits by each code
 * when coding the nb_vals values in arr
 */
void comp_enc(uint64_t *arr, uint64_t nb_vals, char *path)
{
  uint64_t i;
  char *command;
  FILE *file;
  uint8_t ** arrlist;

  command = malloc(sizeof(char) * (316 + 6 * strlen(path)));

  file = fopen(path, "w");
  arrlist = malloc(sizeof(uint8_t) * 6);
  for (i = 0; i < 6; i++)
    arrlist[i] = make_byte_array();

  uint64_t u1, u2, u3, u4, u5, u6;
  fprintf(file, "int  gamma  delta  nibble  zeta_2  zeta_3  zeta_4\n");
  for (i = 1; i <= nb_vals; i++) {
    fprintf(stderr, "%lu \n", arr[i]);
    u1 = 0;
    u2 = 0;
    u3 = 0;
    u4 = 0;
    u5 = 0;
    u6 = 0;

    arrlist[0] = gamma_encoding(arrlist[0], &u1, arr[i]);
    arrlist[1] = delta_encoding(arrlist[1], &u2, arr[i]);
    arrlist[2] = nibble_encoding(arrlist[2], &u3, arr[i]);
    setK(2);
    arrlist[3] = zeta_k_encoding(arrlist[3], &u4, arr[i]);
    setK(3);
    arrlist[4] = zeta_k_encoding(arrlist[4], &u5, arr[i]);
    setK(4);
    arrlist[5] = zeta_k_encoding(arrlist[5], &u6, arr[i]);

    fprintf(file, " %lu  %lu  %lu  %lu  %lu  %lu  %lu\n", arr[i],
      u1, u2, u3, u4, u5, u6);
  }
  fclose(file);
  sprintf(command, "gnuplot -p -e \"set grid; \
    set xlabel 'valeur'; \
    set ylabel 'nombre de bits utilisés pour coder'; \
    set key outside; \
    plot '%s' u 1:2 w lp t 'gamma encoding', \
        '%s' u 1:3 w lp t 'delta encoding', \
        '%s' u 1:4 w lp t 'nibble encoding', \
        '%s' u 1:5 w lp t 'zeta_2', \
        '%s' u 1:6 w lp t 'zeta_3', \
        '%s' u 1:7 w lp t 'zeta_4'\"\
        ", path, path, path, path, path, path);
  assert(system(command) != -1);
  for (i = 0; i < 6; i++)
    free(arrlist[i]);
  free(arrlist);
  free(command);
}

/**
 * generates an array of uint64_t going from 0 to nb_vals,
 * and calls comp_enc on it
 */
void cmp_incr_int_sec(uint64_t nb_vals, char *path) {
  uint64_t i, *arr = malloc(nb_vals * sizeof(uint64_t));
  for (i = 0; i < nb_vals; i++)
    arr[i] = i;
  comp_enc(arr, nb_vals, path);
}

double exp_val_intl(uint64_t *arr, uint64_t nbvals, uint64_t expl_function(uint64_t val)) {
  double res = 0.;
  uint64_t mv = 0, i, * fl, *expll;

  for (i = 0; i < nbvals; i++)
    if (arr[i] > mv)
      mv = arr[i];

  fl = calloc(mv + 1, sizeof(uint64_t));
  expll = calloc(mv + 1, sizeof(uint64_t));

  for (i = 0; i < nbvals; i++)
    fl[arr[i]]++;

  for (i = 0; i < nbvals; i++)
    if (fl[arr[i]] > 0 && expll[arr[i]] == 0)
      expll[arr[i]] = expl_function(arr[i]);

  for (i = 0; i < mv + 1; i++)
    if (fl[i] > 0)
      res += (((double) fl[i]) / nbvals) * expll[i];
  free(fl);
  free(expll);
  return res;
}

void exp_vals_uniform(uint64_t nbvals, uint64_t upperbound, char *filepath)
{
  FILE *data = fopen(filepath, "w");
  uint64_t *arr;
  double ev_gamma, ev_delta, ev_nibble, ev_z2, ev_z3, ev_z4, ev_z5, alpha;

  fprintf(data, "alpha gamma delta nibble zeta_2 zeta_3 zeta_4 zeta_5\n");
  for (uint64_t i = 0; i <= upperbound; i += upperbound / 10) {
    alpha = 1. + 0.1 * i;
    arr = uniform_dist_ints(nbvals, upperbound);
    ev_gamma = exp_val_intl(arr, nbvals, gamma_expl);
    ev_delta = exp_val_intl(arr, nbvals, delta_expl);
    ev_nibble = exp_val_intl(arr, nbvals, nibble_expl);
    setK(2);
    ev_z2 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(3);
    ev_z3 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(4);
    ev_z4 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(5);
    ev_z5 = exp_val_intl(arr, nbvals, zeta_k_expl);
    fprintf(data, "%lf %lf %lf %lf %lf %lf %lf %lf\n",
      alpha, ev_gamma, ev_delta, ev_nibble,
      ev_z2, ev_z3, ev_z4, ev_z5);
    free(arr);
  }
  fclose(data);
  char *command = malloc(sizeof(char) * 1000);
  sprintf(command, "gnuplot -p -e \"set grid; \
    set xlabel ' '; \
    set ylabel 'expected length (number of bits)'; set terminal wxt size 900,600;\
    set key outside; \
    plot '%s' u 1:2 w l t 'gamma encoding', \
        '%s' u 1:3 w l t 'delta encoding', \
        '%s' u 1:4 w l t 'nibble encoding', \
        '%s' u 1:5 w l t 'zeta_2', \
        '%s' u 1:6 w l t 'zeta_3', \
        '%s' u 1:7 w l t 'zeta_4', \
        '%s' u 1:8 w l t 'zeta_5'\" \
        ", filepath, filepath, filepath, filepath, filepath, filepath, filepath);
  assert(system(command) != -1);
  free(command);
}

void exp_vals_power(uint64_t nbvals, uint64_t mv, char *filepath)
{
  FILE *data = fopen(filepath, "w");
  uint64_t *arr;
  double ev_gamma, ev_delta, ev_nibble, ev_z2, ev_z3, ev_z4, ev_z5, alpha;
  fprintf(data, "alpha gamma delta nibble zeta_2 zeta_3 zeta_4 zeta_5\n");
  for (alpha = 1.; alpha <= 2.6; alpha += 0.05) {
    //*i;
    arr = power_dist_ints(nbvals, alpha, mv);
    ev_gamma = exp_val_intl(arr, nbvals, gamma_expl);
    ev_delta = exp_val_intl(arr, nbvals, delta_expl);
    ev_nibble = exp_val_intl(arr, nbvals, nibble_expl);
    setK(2);
    ev_z2 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(3);
    ev_z3 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(4);
    ev_z4 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(5);
    ev_z5 = exp_val_intl(arr, nbvals, zeta_k_expl);
    fprintf(data, "%lf %lf %lf %lf %lf %lf %lf %lf\n",
      alpha, ev_gamma, ev_delta, ev_nibble,
      ev_z2, ev_z3, ev_z4, ev_z5);
    free(arr);
  }
  fclose(data);
  // set label 11 center at graph 0.5,char 1 'Figure 3' font ',14';
  char *command = malloc(sizeof(char) * 1000);
  sprintf(command, "gnuplot -p -e \"set grid; \
    set xlabel 'Zipfs law distribution paramater alpha'; \
    set ylabel 'Expected length (number of bits)';\
    set bmargin 5;\
    set key outside; \
    set terminal png size 900,600; set output 'f3.png';\
    plot '%s' u 1:2 w l t 'gamma encoding', \
        '%s' u 1:3 w l t 'delta encoding', \
        '%s' u 1:4 w l t 'nibble encoding', \
        '%s' u 1:5 w l t 'zeta_2', \
        '%s' u 1:6 w l t 'zeta_3', \
        '%s' u 1:7 w l t 'zeta_4', \
        '%s' u 1:8 w l t 'zeta_5'\" \
        ", filepath, filepath, filepath, filepath, filepath, filepath, filepath);
  assert(system(command) != -1);
  free(command);
}

void exp_vals_poisson(uint64_t nbvals, char *filepath)
{
  FILE *data = fopen(filepath, "w");
  uint64_t *arr;
  double ev_gamma, ev_delta, ev_nibble, ev_z2, ev_z3, ev_z4, ev_z5, lambda;
  fprintf(data, "lambda gamma delta nibble zeta_2 zeta_3 zeta_4 zeta_5\n");
  for (lambda = 10; lambda <= 1000; lambda = +10) {
    arr = poisson_dist_ints(nbvals, lambda);
    ev_gamma = exp_val_intl(arr, nbvals, gamma_expl);
    ev_delta = exp_val_intl(arr, nbvals, delta_expl);
    ev_nibble = exp_val_intl(arr, nbvals, nibble_expl);
    setK(2);
    ev_z2 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(3);
    ev_z3 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(4);
    ev_z4 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(5);
    ev_z5 = exp_val_intl(arr, nbvals, zeta_k_expl);
    fprintf(data, "%lf %lf %lf %lf %lf %lf %lf %lf\n",
      lambda, ev_gamma, ev_delta, ev_nibble,
      ev_z2, ev_z3, ev_z4, ev_z5);
    free(arr);
  }
  fclose(data);
  //set label 11 center at graph 0.5,char 1 'Figure 2' font ',14';
  char *command = malloc(sizeof(char) * 1000);
  sprintf(command, "gnuplot -p -e \"set grid; \
    set xlabel 'Poisson distribution parameter lambda(n=10000)'; \
    set ylabel 'Expected length (number of bits)'; \
    set bmargin 5;\
    set key outside; \
    set terminal png size 900,600; set output 'f2.png';\
    plot '%s' u 1:2 w l t 'gamma encoding', \
        '%s' u 1:3 w l t 'delta encoding', \
        '%s' u 1:4 w l t 'nibble encoding', \
        '%s' u 1:5 w l t 'zeta_2', \
        '%s' u 1:6 w l t 'zeta_3', \
        '%s' u 1:7 w l t 'zeta_4', \
        '%s' u 1:8 w l t 'zeta_5'\" \
        ", filepath, filepath, filepath, filepath, filepath, filepath, filepath);
  assert(system(command) != -1);
  free(command);
}

void exp_vals_binomial(uint64_t nbvals, char *filepath)
{
  FILE *data = fopen(filepath, "w");
  uint64_t *arr, n = 1000;
  double ev_gamma, ev_delta, ev_nibble, ev_z2, ev_z3, ev_z4, ev_z5, p;
  fprintf(data, "p gamma delta nibble zeta_2 zeta_3 zeta_4 zeta_5\n");
  for (p = 0.1; p < 1; p += 0.1) {
    arr = binomial_dist_ints(nbvals, n, p);
    ev_gamma = exp_val_intl(arr, nbvals, gamma_expl);
    ev_delta = exp_val_intl(arr, nbvals, delta_expl);
    ev_nibble = exp_val_intl(arr, nbvals, nibble_expl);
    setK(2);
    ev_z2 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(3);
    ev_z3 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(4);
    ev_z4 = exp_val_intl(arr, nbvals, zeta_k_expl);
    setK(5);
    ev_z5 = exp_val_intl(arr, nbvals, zeta_k_expl);
    fprintf(data, "%lf %lf %lf %lf %lf %lf %lf %lf\n",
      p, ev_gamma, ev_delta, ev_nibble,
      ev_z2, ev_z3, ev_z4, ev_z5);
    free(arr);
  }

  fclose(data);
  //set label 11 center at graph 0.5,char 1 'Figure 1' font ',14';
  char *command = malloc(sizeof(char) * 1000);
  sprintf(command, "gnuplot -p -e \"set grid; \
    set xlabel 'Binomial distribution paramater p (with n=10000)'; \
    set ylabel 'Expected length (number of bits)'; set terminal wxt size 900,600;\
    set bmargin 5;\
    set key outside; \
    set terminal png size 900,600; set output 'f1.png';\
    plot '%s' u 1:2 w l t 'gamma encoding', \
        '%s' u 1:3 w l t 'delta encoding', \
        '%s' u 1:4 w l t 'nibble encoding', \
        '%s' u 1:5 w l t 'zeta_2', \
        '%s' u 1:6 w l t 'zeta_3', \
        '%s' u 1:7 w l t 'zeta_4', \
        '%s' u 1:8 w l t 'zeta_5'\" \
        ", filepath, filepath, filepath, filepath, filepath, filepath, filepath);
  assert(system(command) != -1);
  free(command);
}