#include <stdbool.h>
#include <stdint.h>
#include "matrix_utils.h"
#include "../include/ellpack.h"
#ifndef MATMUL_H
#define MATMUL_H

void matr_mult_ellpack(const void *a, const void *b, void *result);
void matr_mult_ellpack_unsorted(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns);
void matr_mult_ellpack_main_no_simd(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns);
void matr_mult_ellpack_main_simd(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns);

#endif // MATMUL_H
