#ifndef ELLPACK_H
#define ELLPACK_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct
{
    uint64_t nr_rows;
    uint64_t nr_cols;
    uint64_t nr_ellpack_elts; // max # of non zero elements per column -> (<= 2^64 -1)
    float *values;
    bool sorted;
    uint64_t *indices;
    uint64_t *nr_of_non_zeros_per_col; // each number is for the corresponding col
    uint64_t total_non_zero_nr;        // in the whole matrix
} ELLPACKMatrix;

typedef struct {
    uint64_t nr_rows;
    uint64_t nr_cols;
    uint64_t nr_ellpack_elts; // max # of non zero elements per column -> (<= 2^64 -1)
    const float *values;
    bool sorted;
    const uint64_t *indices;
    const uint64_t *nr_of_non_zeros_per_col; // each number is for the corresponding col
    uint64_t total_non_zero_nr;        // in the whole matrix
} const_ELLPACKMatrix;


typedef struct
{
    const char *filename;
    FILE* output;
} result_file;


void clean_file_data(uint8_t *initial_star_index, FILE *aptr);
ELLPACKMatrix get_empty_ellpackmatrix();
void clean_matrix_data(ELLPACKMatrix *a);

/** Checks if two matrices can be multiplied using ELLPACK
 * @returns 1 if they can be multiplied, 0 if they cannot be multiplied, 2 if one of the matrices are wider than 2^32 (should only be handled if one is completely empty)
 */
int check_ellpack_multiplication(const const_ELLPACKMatrix *a, const const_ELLPACKMatrix *b, bool sorted);

void print_ellpack_matrix_2(ELLPACKMatrix *a);

#endif // ELLPACK_H
