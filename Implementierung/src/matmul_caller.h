/**
 * @file matmul_caller.h
 * @brief High-level entry that wires I/O, benchmarking, and matmul.
 */
#include "matrix_utils.h"

/**
 * @brief Read two ELLPACK matrices, multiply them, optionally benchmark, and write result.
 * @param filename_a Path to the first input matrix (ELLPACK format).
 * @param filename_b Path to the second input matrix (ELLPACK format).
 * @param output_file Output path for the result (optional; NULL to skip writing).
 * @param benchmark_iterations Number of repetitions for benchmarking (0 disables benchmarking).
 * @param matmul Matmul implementation to use.
 * @return 0 on success; non-zero on I/O or computation failure.
 */
int call_matmul(const char* filename_a, const char* filename_b, const char* output_file, int benchmark_iterations, matmul_func matmul);