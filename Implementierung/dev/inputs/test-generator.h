#ifndef TEST_GENERATOR_H
#define TEST_GENERATOR_H

#include <stdint.h>

float** allocate_matrix(uint64_t rows, uint64_t cols);
void deallocate_matrix(float **matrix, uint64_t rows);
void print_matrix(uint64_t rows, uint64_t cols, float **matrix);
void generate_sparse_matrix(uint64_t rows, uint64_t cols, float **matrix, float sparsity);
float** multiply_matrices(uint64_t rowsA, uint64_t colsA, float **matrixA, uint64_t rowsB, uint64_t colsB, float **matrixB);
void create_zero_matrix(uint64_t rows, uint64_t cols, float **matrix);

#endif // TEST_GENERATOR_H
