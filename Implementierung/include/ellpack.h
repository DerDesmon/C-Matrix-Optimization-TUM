/**
 * @file ellpack.h
 * @brief Types and helpers for sparse matrices in ELLPACK (column-major) format.
 *
 * This header defines the core data structures used across the project to
 * represent sparse matrices in ELLPACK format and declares validation and
 * utility functions used by I/O and multiplication routines.
 */
#ifndef ELLPACK_H
#define ELLPACK_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * @struct ELLPACKMatrix
 * @brief Mutable ELLPACK matrix in column-major layout.
 *
 * - `values` and `indices` store up to `nr_ellpack_elts` entries per column.
 * - `nr_of_non_zeros_per_col` holds the actual count for each column.
 * - When `sorted` is true, indices within columns are in ascending order.
 */
typedef struct
{
    uint64_t nr_rows;
    uint64_t nr_cols;
    uint64_t nr_ellpack_elts; ///< Max non-zero elements per column
    float *values;
    bool sorted;
    uint64_t *indices;
    uint64_t *nr_of_non_zeros_per_col; ///< Non-zero count per column
    uint64_t total_non_zero_nr;        ///< Total non-zeros in matrix
} ELLPACKMatrix;

/**
 * @struct const_ELLPACKMatrix
 * @brief Read-only view of an ELLPACK matrix.
 */
typedef struct {
    uint64_t nr_rows;
    uint64_t nr_cols;
    uint64_t nr_ellpack_elts; ///< Max non-zero elements per column
    const float *values;
    bool sorted;
    const uint64_t *indices;
    const uint64_t *nr_of_non_zeros_per_col; ///< Non-zero count per column
    uint64_t total_non_zero_nr;        ///< Total non-zeros in matrix
} const_ELLPACKMatrix;


/**
 * @struct result_file
 * @brief Output handle for writing matrices/results to disk.
 */
typedef struct
{
    const char *filename;
    FILE* output;
} result_file;


/**
 * @brief Reset parsing state for input files.
 * @param initial_star_index Pointer to the first '*' marker index buffer.
 * @param aptr Open file pointer to clear/rewind as needed.
 */
void clean_file_data(uint8_t *initial_star_index, FILE *aptr);

/**
 * @brief Create a zero-initialized ELLPACK matrix with no allocated buffers.
 */
ELLPACKMatrix get_empty_ellpackmatrix();

/**
 * @brief Free all allocated buffers and reset the matrix to empty state.
 * @param a Matrix to clean.
 */
void clean_matrix_data(ELLPACKMatrix *a);

/**
 * @brief Check if two matrices can be multiplied in ELLPACK format.
 * @param a Left operand (A) in const ELLPACK view.
 * @param b Right operand (B) in const ELLPACK view.
 * @param sorted Assume sorted indices within columns for both matrices.
 * @return 1 if compatible, 0 if incompatible, 2 if width exceeds 2^32 (only valid when effectively empty).
 */
int check_ellpack_multiplication(const const_ELLPACKMatrix *a, const const_ELLPACKMatrix *b, bool sorted);

/**
 * @brief Debug-print an ELLPACK matrix to stdout.
 */
void print_ellpack_matrix_2(ELLPACKMatrix *a);

#endif // ELLPACK_H
