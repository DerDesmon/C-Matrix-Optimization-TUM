#include <stdlib.h>
#include <stdio.h>
#include "matrix_utils.h"
#include <stdbool.h>
#include <inttypes.h>
#include "io.h"

/**
 * Initializes a result_mat for performing matrix multiplication on a * b 
 * @param initial_col_height The initial height of the columns of the result matrix
 */
result_mat malloc_init_result_mat_from_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b)
{
    uint64_t initial_size = 1;
    if(a->nr_cols > 0) {
        uint64_t avg_non_zeroes_a = (a->total_non_zero_nr / a->nr_cols) / 10; // divide by 10 to get a tenth of the average non-zeroes per column (conservative estimate)
        if(avg_non_zeroes_a > initial_size) initial_size = avg_non_zeroes_a; 
    }
    return malloc_init_result_mat(b->nr_cols, initial_size, a->nr_rows);
}
/**
 * Initializes a result_mat and mallocs space for nested array. Should not be freed in case of failure.
 * @returns returns the 128-bit result_mat with initialized memory. If any initialization malloc failed, the `cols` will be set to NULL
 */
result_mat malloc_init_result_mat(unsigned int column_amount, unsigned int initial_col_height, uint64_t max_col_height)
{
    // column_amount = nr of result_cols needed = cols_len
    result_mat matrix = {
        .cols = malloc(sizeof(result_col) * column_amount),
        .cols_len = column_amount,
        .max_col_height = max_col_height > UINT32_MAX ? UINT32_MAX : max_col_height, // limit size to UINT32_MAX since an array of size 2^64 is not feasible
    };

    if (matrix.cols == NULL)
    {
        fprintf(stderr, "Could not allocate space for the result_mat!");
        return matrix; // has cols as NULL, indicating failure
    }

    bool error_occured = false;
    // allocate the result rows of the matrix -> each row has to be either allocated or nullptr for cleanup
    for (unsigned int i = 0; i < column_amount; i++)
    {
        matrix.cols[i].used_height = 0;
        matrix.cols[i].height = initial_col_height;
        uint64_t *indices = error_occured ? NULL : malloc(sizeof(uint64_t) * initial_col_height);
        float *values = error_occured ? NULL : malloc(sizeof(float) * initial_col_height);

        // allocate indices array
        if (indices == NULL || values == NULL)
        {
            if (!error_occured)
                fprintf(stderr, "Could not allocate space for the result indices or values!");
            error_occured = true;
        }

        // on failure, will be allocated with NULL and then later be ignored by free()
        matrix.cols[i].indices = indices;
        matrix.cols[i].values = values;
    }

    if (error_occured)
    {
        free_result_mat(&matrix); // has cols as NULL, indicating failure
    }

    return matrix;
};
/* returns 0 if successful, else 1 if failed. If failed, the result_mat still needs to be cleaned/freed */
int push_to_matrix(result_mat *mat, float value, uint64_t i_row, unsigned int i_col)
{
    // find Col
    if (i_col >= mat->cols_len)
    {
        fprintf(stderr, "Col index of value was bigger than the matrix!");
        return EXIT_FAILURE;
    }
    result_col *col = &mat->cols[i_col];
    // enough space? (used_height is starting from 0 as a index, pointing to the nr of elts inside, col->height "is counting the number of values that would fit into")
    if (col->used_height >= col->height)
    { // no space -> resize
        unsigned int new_height = col->height * 2; // double size to achieve amortized O(1) time complexity
        new_height = new_height > mat->max_col_height ? mat->max_col_height : new_height; // limit size to max_col_height -> saves memory

        // in case of overflow or resize beyond max size, print to standard error that the matrix is too big and return error response
        if (new_height <= col->height || col->height == mat->max_col_height)
        {
            fprintf(stderr, "Matrix is too big to be resized!\n");
            return EXIT_FAILURE;
        }

        float *new_values = realloc(col->values, new_height * sizeof(float));
        if (!new_values) {
            fprintf(stderr, "Failed to reallocate memory for values!");
            return EXIT_FAILURE;
        }

        uint64_t *new_indices = realloc(col->indices, new_height * sizeof(uint64_t));
        if (!new_indices) {
            fprintf(stderr, "Failed to reallocate memory for indices!");
            col->values = new_values; // the old values array was already freed, so we need to keep the new one to be cleaned up later
            return EXIT_FAILURE;
        }
        else
        {
            col->values = new_values;
            col->indices = new_indices;
            col->height = new_height;
        }
    }
    // enough space -> insert & set index further
    col->values[col->used_height] = value;
    col->indices[col->used_height++] = i_row;
    return EXIT_SUCCESS;
};

void free_result_mat(result_mat *matrix)
{
    if (matrix->cols == NULL)
        return; // freeing an already freed matrix
    unsigned int col_amount = matrix->cols_len;

    for (unsigned int i = 0; i < col_amount; i++)
    {
        // free each col
        free(matrix->cols[i].indices);
        free(matrix->cols[i].values);
    }
    // and whole matrix
    free(matrix->cols);
    matrix->cols = NULL;
};

unsigned int get_longest_col(result_mat *matrix)
{
    unsigned int longest = 0;
    for (unsigned int i = 0; i < matrix->cols_len; i++)
    {
        if (matrix->cols[i].used_height > longest)
        {
            longest = matrix->cols[i].used_height;
        }
    }
    return longest;
};

//TODO: remove, this is a debug method
void print_ellpack_matrix(result_mat *matrix) {
    printf("Matrix has %u columns\n", matrix->cols_len);
    printf("cols: %p\n", matrix->cols);

    for (unsigned int i = 0; i < matrix->cols_len; i++) {
        printf("col %u: %p\n", i, matrix->cols);
        result_col col = matrix->cols[i];
        printf("col %u has %u elements\n", i, col.used_height);
        for (unsigned int j = 0; j < col.used_height; j++) {
            printf("index %u: %"PRIu64", value %f\n", j, col.indices[j], col.values[j]);
        }
    }
}