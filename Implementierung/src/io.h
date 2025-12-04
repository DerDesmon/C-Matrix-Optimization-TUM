/**
 * @file io.h
 * @brief Read and write ELLPACK matrices and computed results.
 */
#ifndef IO_H
#define IO_H
#include <stdbool.h>
#include "../include/ellpack.h"
#include "matrix_utils.h"

/**
 * @brief Read an ELLPACK matrix from a file.
 * @param a Output matrix (buffers allocated inside on success).
 * @param filename Path to the input file.
 * @return 0 on success, non-zero on parse/IO error.
 */
int read_ellpack_matrix(ELLPACKMatrix *a, const char *filename);

/**
 * @brief Write a result matrix to a file in ELLPACK-like layout.
 * @param filename Output file path.
 * @param matrix Result matrix to serialize.
 * @param rows Number of rows in the original result grid.
 * @param cols Number of columns in the original result grid.
 * @param error_occurred_in_write Optional error flag; set to true on failure.
 * @return A `result_file` handle (NULL on failure).
 */
result_file *write_ellpack_matrix(const char *filename, result_mat *matrix, uint64_t rows, uint64_t cols, bool *error_occurred_in_write);

#endif // IO_H