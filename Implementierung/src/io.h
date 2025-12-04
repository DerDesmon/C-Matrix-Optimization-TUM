#ifndef IO_H
#define IO_H
#include <stdbool.h>
#include "../include/ellpack.h"
#include "matrix_utils.h"

int read_ellpack_matrix(ELLPACKMatrix *a, const char *filename);
result_file *write_ellpack_matrix(const char *filename, result_mat *matrix, uint64_t rows, uint64_t cols, bool *error_occurred_in_write);

#endif // IO_H