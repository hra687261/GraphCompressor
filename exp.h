#ifndef EXPERIMENTATIONS
#define EXPERIMENTATIONS

#include "integerEncoding.h"
#include "integerGeneration.h"
    
    void idist_to_file(uint64_t *arr, uint64_t nb_vals, char *path);

    void plot_idfile(char *file_path);

    void encoding_decoding_tests(uint64_t nb_vals);


    void test_uniform(uint64_t nb_vals, char *path, uint64_t max);

    void test_poisson(uint64_t nb_vals, char *path, double lambda);

    void test_power(uint64_t nb_vals, char *path, double alpha, uint64_t n);
   
    void test_binomial(uint64_t nb_vals, char *path, uint64_t n, double p);


    void comp_enc(uint64_t *arr, uint64_t nb_vals, char *path);

    void cmp_incr_int_sec(uint64_t nb_vals, char *path);

    double exp_val_intl(uint64_t *arr, uint64_t nbvals, uint64_t expl_function(uint64_t val));

 
    void exp_vals_uniform(uint64_t nbvals, uint64_t upperbound, char *filepath);

    void exp_vals_power(uint64_t nbvals, uint64_t mv, char *filepath);

    void exp_vals_poisson(uint64_t nbvals, char *filepath);

    void exp_vals_binomial(uint64_t nbvals, char *filepath);

#endif