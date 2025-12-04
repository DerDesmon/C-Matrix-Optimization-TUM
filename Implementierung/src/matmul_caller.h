#include "matrix_utils.h"

/** A function that reads two matrices from files, multiplies them and writes the result to a file.
 * @param filename_a The file to read the first matrix from
 * @param filename_b The file to read the second matrix from
 * @param output_file The file to write the result to. If NULL, the result will not be written to a file
 * @param benchmark_iterations How often the multiplication should be repeated for benchmarking (0 for no benchmarking)
 * @param matmul The matmul function to usex
 */
int call_matmul(const char* filename_a, const char* filename_b, const char* output_file, int benchmark_iterations, matmul_func matmul);