#include "../include/ellpack.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "matmul.h"
#include <inttypes.h>
#include "matrix_utils.h"
#include <math.h>
#include <emmintrin.h>
#include <smmintrin.h>
#define EPSILON 1e-7f

void matr_mult_ellpack_unsorted(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns);
// actual function
void matr_mult_ellpack_main(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns, const bool simd);
// wrapper without simd
void matr_mult_ellpack_main_no_simd(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns){
    matr_mult_ellpack_main(matr_a, matr_b, result_columns, false);
}
// wrapper with simd
void matr_mult_ellpack_main_simd(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns){
    matr_mult_ellpack_main(matr_a, matr_b, result_columns, true);
}

/**
 * Implementation for unsorted ellpack matrices
 * @param a Pointer to an ELLPACKMatrix with the first matrix data
 * @param b Pointer to an ELLPACKMatrix with the second matrix data
 * @param result Pointer to a result_mat struct allocated with malloc_init_result_mat. On error, will be "freed" and the cols pointer will be NULL
 */
void matr_mult_ellpack(const void *a, const void *b, void *result)
{
    bool sorted = (((const_ELLPACKMatrix*) a)->sorted) && (((const_ELLPACKMatrix*) b)->sorted);
    if(sorted) {
        matr_mult_ellpack_main_simd((const_ELLPACKMatrix*) a,(const_ELLPACKMatrix*)  b, (result_mat*) result);
    } else {
        printf("Warning: Unsorted indices detected, using slower unsorted algorithm\n");
        matr_mult_ellpack_unsorted((const_ELLPACKMatrix*) a,(const_ELLPACKMatrix*)  b, (result_mat*) result);
    }
}
/** Helper for the next_row function that finds the lowest row index in the matrix
 */
uint64_t find_lowest_row_idx(const const_ELLPACKMatrix* mat, const uint64_t* start_indices, bool* found) {
    *found = false;
    const uint64_t* indices = mat->indices;
    const uint64_t* col_sizes = mat->nr_of_non_zeros_per_col;
    uint64_t result = 0;
    // i is index in the indices array (in the entire matrix)
    for (uint64_t col_idx = 0, i = 0; col_idx < mat->nr_cols; col_idx++)
    {
        uint64_t col_size = col_sizes[col_idx]; // how many elements the current column has
        uint64_t column_offset = start_indices[col_idx]; // the offset of the first unparsed element in the column
        if(column_offset < col_size) {
            uint64_t row_idx = indices[i + column_offset];
            if(*found == false || row_idx < result){
                result = row_idx; //found a new lowest row index
                *found = true;
            }
        }
        i += col_size; // move to the next column
    }

    return result;
}

/**
 * Helper that parses the next row of a matrix into the array pointer passed (state is kept in start_indices)
 * @param result_ptr: pointer to an array of size cols where the row will be written to
 * @param mat: the matrix we are parsing from (is not modified)
 * @param start_indices: an array of size cols where the start index of each column is stored (for each column, the indices less than this index (exclusive) have already been parsed to a previous row)
 * @param found: a pointer to a boolean that will be set to true if a row was found
 * @returns the found row index
 */
uint64_t next_row(float *result_ptr, const const_ELLPACKMatrix* mat, uint64_t* start_indices, bool* found)
{
    /* optimizations not done:
    - simd doesn't make too much sense here since with the start_indices, we don't need to scan a lot of neighboring elements in each column
    - row major *result_ptr would only be useful in case the matrix is very wide, which is inefficient in column major ellpack anyways
        -> this "dense" format is better for most cases since it offers easy access to the columns of the row
    */

    // an array with data on how many elements each column has -> see format description for more details
    const uint64_t *src_col_sizes = mat->nr_of_non_zeros_per_col;
    // how many columns we have -> is the size of the src_col_sizes array and the result_ptr array
    uint64_t cols = mat->nr_cols;
    // ellpack indices from src matrix
    const uint64_t *src_indices = mat->indices;
    // ellpack values from src matrix
    const float *src_values = mat->values;


    uint64_t row_idx = find_lowest_row_idx(mat, start_indices, found); //found is set to true if we found a row

    if(!*found) {
        return 0; //invalid because of the found check
    }

    // i is index in the src values/indices arrays (in the entire matrix)
    for (uint32_t col_idx = 0, i = 0; col_idx < cols; col_idx++) //this is 32 because it is checked in matrix validator
    {
        uint64_t col_size = src_col_sizes[col_idx]; // how many elements the current column has
        uint64_t col_offset = start_indices[col_idx];

        float col_result = 0;
        int matrix_index = i + col_offset;
        if(col_size > 0 && col_offset < col_size && src_indices[matrix_index] == row_idx){
            col_result = src_values[matrix_index];
            start_indices[col_idx]++; // store that we found a value in this column
        }
        result_ptr[col_idx] = col_result; // this is always executed and fills the array with 0s if necessary

        i += col_size;
    }
    return row_idx;
}

static inline float dot_product(const float *row_cache, const uint64_t *col_indices, const float *col_values, const uint64_t col_start_idx, const uint64_t col_num_elts) {
    float result_value = 0;
    // for each element in the column
    for (uint64_t b_col_offset = 0; b_col_offset < col_num_elts; b_col_offset++) {
        uint64_t ellpack_idx_in_b = col_start_idx + b_col_offset;
        float row_value = row_cache[col_indices[ellpack_idx_in_b]];
        result_value += row_value * col_values[ellpack_idx_in_b];
    }
    return result_value;
}

static inline float dot_product_simd(const float *row_cache, const uint64_t *col_indices, const float *col_values, const uint64_t col_start_idx, const uint64_t col_num_elts) {
     __m128 result_vector = _mm_setzero_ps();
     uint64_t b_col_offset = 0;
     // for each element in the column
     for (; b_col_offset + 4 <= col_num_elts; b_col_offset += 4) {

         //step 1: load the corresponding row values into a vector
         uint64_t idx0 = col_start_idx + b_col_offset;
         uint64_t idx1 = col_start_idx + b_col_offset + 1;
         uint64_t idx2 = col_start_idx + b_col_offset + 2;
         uint64_t idx3 = col_start_idx + b_col_offset + 3;

         __m128 row_vector = _mm_set_ps(
             row_cache[col_indices[idx3]],
             row_cache[col_indices[idx2]],
             row_cache[col_indices[idx1]],
             row_cache[col_indices[idx0]]
         );
         //step 2: load the column values into a vector
         __m128 value_vector = _mm_loadu_ps(&col_values[idx0]);
         //step 3: multiply the vectors and add the result to the result vector
         __m128 product_vector = _mm_mul_ps(row_vector, value_vector);
         //step 4: add the product to the result vector
         result_vector = _mm_add_ps(result_vector, product_vector);
     }
     // aggregate the result vector to a single float
     result_vector = _mm_hadd_ps(result_vector, result_vector);
     result_vector = _mm_hadd_ps(result_vector, result_vector);
     float result_value = _mm_cvtss_f32(result_vector);

     // process remaining elements
     for (; b_col_offset < col_num_elts; b_col_offset++) {
         uint64_t ellpack_idx_in_b = col_start_idx + b_col_offset;
         float row_value = row_cache[col_indices[ellpack_idx_in_b]];
         result_value += row_value * col_values[ellpack_idx_in_b];
     }

     return result_value;
}

/**
 * Main implementation, without simd
 * @param matr_a Pointer to an ELLPACKMatrix with the first matrix data
 * @param matr_b Pointer to an ELLPACKMatrix with the second matrix data
 * @param result Pointer to a result_mat struct allocated with malloc_init_result_mat. On error, will be "freed" and the cols pointer will be NULL
 */
void matr_mult_ellpack_main(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns, const bool simd)
{
    //set to null for cleanup
    float *row_cache = NULL;
    uint64_t *start_indices = NULL;

    //check if one of the matrices is completely empty
    if(matr_a->nr_ellpack_elts == 0 || matr_b->nr_ellpack_elts == 0){
        return; //the initial state of result_mat is valid
    }
    
    //check validity of the matrices
    int continue_status = check_ellpack_multiplication(matr_a, matr_b, true);
    if(continue_status == 0) goto cleanup_error;
    else if(continue_status == 2) { //width of one of the matrices is more than UINT32_MAX
        fprintf(stderr, "Multiplication aborted due to wide columns\n");
        goto cleanup_error;
    }

    const uint32_t matr_b_cols = matr_b->nr_cols;

    /* init work: all variables with data that has to be freed */
    /** row cache: since reading a row from a column-major */    
    row_cache = malloc(matr_a->nr_cols * sizeof(float));
    if (row_cache == NULL)
    {
        fprintf(stderr, "Could not allocate row cache\n");
        goto cleanup_error;
    }

    //an array to make find_row more efficient
    start_indices = calloc(matr_a->nr_cols, sizeof(uint64_t));
    if (start_indices == NULL)
    {
        fprintf(stderr, "Could not allocate start indices\n");
        goto cleanup_error;
    }
    /* init end */
    
    bool found = false;
    uint64_t i = next_row(row_cache, matr_a, start_indices, &found);
    // in the following code, i is index in a (row loop), j is index in b (column loop)
    // for each row in a
    while (found)    
    {
        uint64_t b_col_start = 0;

        // for each column -> is uint32_t because this is checked in the matrix validator
        for (uint32_t j = 0; j < matr_b_cols; j++)
        {
            uint64_t num_col_elts = matr_b->nr_of_non_zeros_per_col[j];
            float result_value = 
                simd ? dot_product_simd(row_cache, matr_b->indices, matr_b->values, b_col_start, num_col_elts)
                : dot_product(row_cache, matr_b->indices, matr_b->values, b_col_start, num_col_elts);
            b_col_start += num_col_elts;

            if (!(fabs(result_value) < EPSILON)) //for result_value == 0, we do not need to store it due to ellpack
            {
                // push the result to the result matrix -> error is extremely unlikely here
                bool push_status = __builtin_expect(push_to_matrix(result_columns, result_value, i, j) == EXIT_FAILURE, 0);
                if(push_status == EXIT_FAILURE){
                    goto cleanup_error;
                }
            }
        }
        i = next_row(row_cache, matr_a, start_indices, &found);
    }

    goto cleanup;
cleanup_error:
    free_result_mat(result_columns); //sets the cols pointer to NULL, indicating that an error occured and result_mat cannot be written to the file
cleanup:
    free(start_indices);
    free(row_cache);
}

/* Unsorted implementation 
Some of the code here is duplicated from the sorted implementation, but the code is not easily reusable due to the different structure of the matrices and assumptions
*/

/**
 * Helper that parses the ith row of a matrix and also works with unsorted indices. Is very inefficient but also works for unsorted indices.
 * @param row_idx: the row we are parsing
 * @param result_ptr: pointer to an array of size cols where the row will be written to
 * @param mat: the matrix we are parsing from (is not modified)
 * @param start_indices: an array of size cols where the start index of each column is stored (for each column, the indices below this index have already been parsed to a previous row)
 * @returns 0 (false) if no elements were found in this row (entire row multiplication can be skipped) for this row
 */
bool get_row(float *result_ptr, uint64_t row_idx, const const_ELLPACKMatrix* mat)
{
    // an array with data on how many elements each column has -> see format description for more details
    const uint64_t *src_col_sizes = mat->nr_of_non_zeros_per_col;
    // how many columns we have -> is the size of the src_col_sizes array and the result_ptr array
    uint64_t cols = mat->nr_cols;
    // ellpack indices from src matrix
    const uint64_t *src_indices = mat->indices;
    // ellpack values from src matrix
    const float *src_values = mat->values;

    int i = 0; // i is index in the src values/indices arrays (in the entire matrix)
    bool found_value = false;
    for (uint64_t col_idx = 0; col_idx < cols; col_idx++)
    {
        uint64_t col_size = src_col_sizes[col_idx]; // how many elements the current column has

        float col_result = 0.0f;

        // for an empty column (only 0s), this will just be skipped
        uint64_t j = 0;
        for (; j < col_size; j++)
        {
            int matrix_index = i + j;
            // compare if we are in the correct row, if we are then we have found the result
            if (src_indices[matrix_index] == row_idx)
            {
                found_value = true;
                col_result = src_values[matrix_index];
                break;
            }
        }
        result_ptr[col_idx] = col_result; // this is always executed and fills the array with 0s if necessary

        i += col_size;
    }
    return found_value;
}

/**
 * Implementation for unsorted ellpack matrices
 * @param matr_a Pointer to an ELLPACKMatrix with the first matrix data
 * @param matr_b Pointer to an ELLPACKMatrix with the second matrix data
 * @param result Pointer to a result_mat struct allocated with malloc_init_result_mat. On error, will be "freed" and the cols pointer will be NULL
 */
void matr_mult_ellpack_unsorted(const const_ELLPACKMatrix *matr_a, const const_ELLPACKMatrix *matr_b, result_mat *result_columns)
{
    //set to null for cleanup
    float *row_cache = NULL;
    uint64_t *start_indices = NULL;
    
    //check validity of the matrices
    printf("Checking matrices\n");
    int continue_status = check_ellpack_multiplication(matr_a, matr_b, false);
    if(continue_status == 0) goto cleanup_error;
    else if(continue_status == 2) { //width of one of the matrices is more than UINT32_MAX
        //check if one of the matrices is completely empty
        if(matr_a->nr_ellpack_elts == 0 || matr_b->nr_ellpack_elts == 0){
            return; //the initial state of result_mat is valid
        }
        else {
            fprintf(stderr, "Multiplication aborted due to wide columns\n");
            goto cleanup_error;
        }
    }

    const uint32_t matr_b_cols = matr_b->nr_cols;

    /* init work: all variables with data that has to be freed */
    /** row cache: since reading a row from a column-major */    
    row_cache = malloc(matr_a->nr_cols * sizeof(float));
    if (row_cache == NULL)
    {
        fprintf(stderr, "Could not allocate row cache\n");
        goto cleanup_error;
    }

    //an array to make find_row more efficient
    start_indices = calloc(matr_a->nr_cols, sizeof(uint64_t));
    if (start_indices == NULL)
    {
        fprintf(stderr, "Could not allocate start indices\n");
        goto cleanup_error;
    }
    /* init end */
    
    // in the following code, i is index in a (row loop), j is index in b (column loop)
    // for each row in a
    for(uint64_t i = 0; i < matr_a->nr_rows; i++)
    {
        uint64_t b_col_start = 0;
        bool found = get_row(row_cache, i, matr_a);
        if(!found) continue; //skip the row if it is empty

        // for each column -> is uint32_t because this is checked in the matrix validator
        for (uint32_t j = 0; j < matr_b_cols; j++)
        {
            float result_value = 0;
            uint64_t num_col_elts = matr_b->nr_of_non_zeros_per_col[j];
            // for each filled ellpack element
            for (uint64_t col_elt = 0; col_elt < num_col_elts; col_elt++)
            {
                uint64_t ellpack_idx_in_b = b_col_start + col_elt;
                float row_value = row_cache[matr_b->indices[ellpack_idx_in_b]];
                result_value += row_value * matr_b->values[ellpack_idx_in_b];
            }
            b_col_start += num_col_elts;

            if (!(fabs(result_value) < EPSILON)) //for result_value == 0, we do not need to store it due to ellpack
            {
                // push the result to the result matrix -> error is extremely unlikely here
                bool push_status = __builtin_expect(push_to_matrix(result_columns, result_value, i, j) == EXIT_FAILURE, 0);
                if(push_status == EXIT_FAILURE){
                    goto cleanup_error;
                }
            }
        }
    }

    goto cleanup;
cleanup_error:
    free_result_mat(result_columns); //sets the cols pointer to NULL, indicating that an error occured and result_mat cannot be written to the file
cleanup:
    free(start_indices);
    free(row_cache);
}

