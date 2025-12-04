#include <stdint.h>
#include "../include/ellpack.h"

#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

// Functions go here that are used for matrix operations in many files

/** result_col struct
 * is a "double" dynamic array of an ellpack column 
 */
typedef struct {
    uint64_t* indices;
    float* values;
    unsigned int height; //is not uint64_t because it is not feasible to store an array of even size 2^32
    unsigned int used_height;
} result_col;

/** result_mat struct
 * has an array of columns, each of which is a dynamic array
 * a valid, allocated result_mat has a valid result_col* pointer. If some memory allocation fails, this pointer should be NULL
 */
typedef struct {
    result_col* cols;
    unsigned int cols_len; //is not uint64_t because it is not feasible to store an array of even size 2^32
    unsigned int max_col_height; //^ same as above; is the maximum height of any column (improves memory efficiency and is free memory here due to alignment)
} result_mat;

/** allocate memory for a result_mat, including initializing the columns */
result_mat malloc_init_result_mat_from_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b);
result_mat malloc_init_result_mat(unsigned int column_amount, unsigned int initial_col_height, uint64_t max_col_height);
int push_to_matrix(result_mat* mat, float value, uint64_t i_row, unsigned int i_col);
void free_result_mat(result_mat* matrix);
unsigned int get_longest_col(result_mat *matrix);
void matr_mult_ellpack(const void *a, const void *b, void *result);

void print_ellpack_matrix(result_mat *a);

typedef void (*matmul_func)(const ELLPACKMatrix* a, const ELLPACKMatrix* b, result_mat* result);

#endif //MATRIX_UTILS_H
