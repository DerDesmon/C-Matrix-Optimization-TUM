#include "../src/matmul.h"
#include "../include/ellpack.h"
#include "../src/matrix_utils.h"

/**
 * A wrapper around a matmul function that benchmarks it.
 * @param n_times How often the benchmark should run
 * @param data_a The first parameter of the matmul function
 * @param data_b The second parameter of the matmul function
 * @param result_mat An initialized result matrix, works the same as in matmul.c
 * @param specifier An identifier for the test (can be NULL)
 * @param matmul The matmul function to benchmark
 */
void run_benchmark(int n_times, ELLPACKMatrix* data_a, ELLPACKMatrix* data_b, result_mat* result_mat, matmul_func matmul);