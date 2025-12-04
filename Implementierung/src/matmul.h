/**
 * @file matmul.h
 * @brief ELLPACK-based sparse matrix multiplication interfaces.
 */
#include <stdbool.h>
#include <stdint.h>
#include "matrix_utils.h"
#include "../include/ellpack.h"
#ifndef MATMUL_H
#define MATMUL_H

/**
 * @brief Generic entry point with opaque pointers (compat layer).
 * @param a Pointer to `const_ELLPACKMatrix` left operand.
 * @param b Pointer to `const_ELLPACKMatrix` right operand.
 * @param result Pointer to `result_mat` output accumulator.
 */
void matr_mult_ellpack(const void *a, const void *b, void *result);

/**
 * @brief Multiply two ELLPACK matrices assuming unsorted column indices.
 * @param matr_a Left operand.
 * @param matr_b Right operand.
 * @param result_columns Accumulator for result columns (pre-initialized).
 */
void matr_mult_ellpack_unsorted(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns);

/**
 * @brief Main multiplication routine without SIMD acceleration.
 * @param matr_a Left operand.
 * @param matr_b Right operand.
 * @param result_columns Accumulator for result columns (pre-initialized).
 */
void matr_mult_ellpack_main_no_simd(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns);

/**
 * @brief Main multiplication routine with SIMD acceleration.
 * @param matr_a Left operand.
 * @param matr_b Right operand.
 * @param result_columns Accumulator for result columns (pre-initialized).
 */
void matr_mult_ellpack_main_simd(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns);

#endif // MATMUL_H
