/**
 * @file matrix_utils.h
 * @brief Utility types and helpers for working with ELLPACK matrices and results.
 */
#include <stdint.h>
#include "../include/ellpack.h"

#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

/**
 * @struct result_col
 * @brief Dynamic column container for the result matrix (ELLPACK-like).
 *
 * Holds parallel arrays of `indices` and `values` with a current `used_height`
 * and a capacity `height` for amortized O(1) append.
 */
typedef struct {
    uint64_t* indices;
    float* values;
    unsigned int height; ///< Capacity of the column buffer
    unsigned int used_height; ///< Number of valid elements
} result_col;

/**
 * @struct result_mat
 * @brief Result accumulator consisting of dynamic columns.
 *
 * `cols_len` equals the number of output columns. `max_col_height` tracks the
 * maximum height among columns to improve memory efficiency.
 */
typedef struct {
    result_col* cols;
    unsigned int cols_len;
    unsigned int max_col_height;
} result_mat;

/**
 * @brief Initialize a `result_mat` sized to the product of two ELLPACK matrices.
 * @param a Left operand.
 * @param b Right operand.
 * @return Allocated result matrix; `cols` is NULL on allocation failure.
 */
result_mat malloc_init_result_mat_from_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b);

/**
 * @brief Initialize a `result_mat` with specified column count and initial capacity.
 * @param column_amount Number of columns in the result.
 * @param initial_col_height Initial capacity per column.
 * @param max_col_height Upper bound used for planning memory layout.
 */
result_mat malloc_init_result_mat(unsigned int column_amount, unsigned int initial_col_height, uint64_t max_col_height);

/**
 * @brief Append a value at row `i_row` to column `i_col` in the result.
 * @param mat Result matrix.
 * @param value Value to append.
 * @param i_row Row index of the value.
 * @param i_col Column index in result.
 * @return 0 on success, non-zero on failure.
 */
int push_to_matrix(result_mat* mat, float value, uint64_t i_row, unsigned int i_col);

/**
 * @brief Free all memory of a result matrix and set fields to safe defaults.
 */
void free_result_mat(result_mat* matrix);

/**
 * @brief Get the height of the longest column in the result matrix.
 */
unsigned int get_longest_col(result_mat *matrix);

/**
 * @brief Compatibility wrapper signature (see matmul.h for implementations).
 */
void matr_mult_ellpack(const void *a, const void *b, void *result);

/**
 * @brief Debug-print a result matrix in ELLPACK-like layout.
 */
void print_ellpack_matrix(result_mat *a);

/**
 * @brief Function pointer type for matmul implementations.
 */
typedef void (*matmul_func)(const ELLPACKMatrix* a, const ELLPACKMatrix* b, result_mat* result);

#endif //MATRIX_UTILS_H
