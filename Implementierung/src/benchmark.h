/**
 * @file benchmark.h
 * @brief Simple benchmarking wrapper for matmul implementations.
 */
#include "../src/matmul.h"
#include "../include/ellpack.h"
#include "../src/matrix_utils.h"

/**
 * @brief Benchmark a matmul function by running it repeatedly.
 * @param n_times Number of repetitions.
 * @param data_a First input matrix.
 * @param data_b Second input matrix.
 * @param result_mat Pre-initialized result matrix accumulator.
 * @param matmul Matmul implementation to benchmark.
 */
void run_benchmark(int n_times, ELLPACKMatrix* data_a, ELLPACKMatrix* data_b, result_mat* result_mat, matmul_func matmul);