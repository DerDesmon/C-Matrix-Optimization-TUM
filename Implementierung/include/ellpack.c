#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "ellpack.h"
#include "../src/matrix_utils.h"

/** Helper that frees memory from an ellpackmatrix */

void clean_file_data(uint8_t *initial_star_index /* , alloc_data *data */, FILE *aptr)
{
    if (aptr)
    {
        fclose(aptr);
    }
    if (initial_star_index)
    {
        free(initial_star_index);
    }
}

ELLPACKMatrix get_empty_ellpackmatrix() {
    ELLPACKMatrix matrix = {
        .nr_rows = 0,
        .nr_cols = 0,
        .nr_ellpack_elts = 0,
        .values = NULL,
        .sorted = false,
        .indices = NULL,
        .nr_of_non_zeros_per_col = NULL,
        .total_non_zero_nr = 0
    };
    return matrix;
}

// cleans the allocated space of the Ellpack matrix, if it was allocated
void clean_matrix_data(ELLPACKMatrix *a)
{
    free(a->values);
    a->values = NULL;
    free(a->indices);
    a->indices = NULL;
    free(a->nr_of_non_zeros_per_col);
    a->nr_of_non_zeros_per_col = NULL;
}

int check_ellpack_multiplication(const const_ELLPACKMatrix *a, const const_ELLPACKMatrix *b, bool should_be_sorted) {
    bool compatible_matrices = a->nr_cols == b->nr_rows;
    if (!compatible_matrices)
    {
        fprintf(stderr, "Incompatible matrices provided :( a has %" PRIu64 " columns, b has %" PRIu64 " rows\n", a->nr_cols, b->nr_rows);
        return 0;
    }
    uint64_t nonzero_a = a->nr_ellpack_elts;
    uint64_t nonzero_b = b->nr_ellpack_elts;
    
    if (nonzero_a != 0 && UINT64_MAX / nonzero_a < nonzero_b)
    {
        fprintf(stderr, "Multiplication between %" PRIu64 " and %" PRIu64 " would overflow", nonzero_a, nonzero_b);
        return 0;
    }

    if(should_be_sorted) { //both matrices should be sorted
        if(!a->sorted || !b->sorted) {
            fprintf(stderr, "Both matrices should be sorted, but status is:\n - a_sorted: %d\n - b_sorted: %d\n", a->sorted, b->sorted);
            return 0;
        }
    }

    //these cases will not be handled since to save even 2 * 2 * 2^32 chars, you would a 16gb file (in ellpack)
    // 2* for `*,`; 2* for the indices; and 2^32 for columns
    if(a->nr_cols > UINT32_MAX) {
        fprintf(stderr, "Warning: matrix a is too wide: %" PRIu64" columns\n", a->nr_cols);
        return 2;
    }

    if(b->nr_cols > UINT32_MAX) {
        fprintf(stderr, "Warning: matrix b is too wide: %" PRIu64" columns\n", b->nr_cols);
        return 2;
    }

    return 1;
}

//TODO: remove, this is a debug function
void print_ellpack_matrix_2(ELLPACKMatrix *a) {
    printf("Matrix with %" PRIu64 " rows, %" PRIu64 " columns, %" PRIu64 " non-zero elements per column\n", a->nr_rows, a->nr_cols, a->nr_ellpack_elts);
    uint64_t idx = 0;
    for (uint64_t i = 0; i < a->nr_cols; i++) {
        printf("Column %" PRIu64 " has %" PRIu64 " non-zero elements\n", i, a->nr_of_non_zeros_per_col[i]);
        for (uint64_t j = 0; j < a->nr_of_non_zeros_per_col[i]; j++) {
            printf("(%f, %" PRIu64 ")", a->values[idx + j], a->indices[idx + j]);
        }
        idx += a->nr_of_non_zeros_per_col[i];
        printf("\n");
    }
}