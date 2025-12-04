#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "test-generator.h"

#define BASE_PATH "dev/inputs/dist/"
#define MAX_FILENAME_LENGTH 256

float** allocate_matrix(uint64_t rows, uint64_t cols) {
    float **matrix = (float **)malloc(rows * sizeof(float *));
    if (matrix == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (uint64_t i = 0; i < rows; i++) {
        matrix[i] = (float *)malloc(cols * sizeof(float));
        if (matrix[i] == NULL) {
            fprintf(stderr, "memory allocation failed\n");
            for (uint64_t j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            exit(EXIT_FAILURE);
        }
    }
    return matrix;
}


void deallocate_matrix(float **matrix, uint64_t rows) {
    for (uint64_t i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void print_matrix(uint64_t rows, uint64_t cols, float **matrix) {
    for (uint64_t i = 0; i < rows; i++) {
        for (uint64_t j = 0; j < cols; j++) {
            printf("%.2f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void generate_sparse_matrix(uint64_t rows, uint64_t cols, float **matrix, float sparsity) {
    srand(time(NULL));
    for (uint64_t i = 0; i < rows; i++) {
        for (uint64_t j = 0; j < cols; j++) {
            if ((rand() / (float)RAND_MAX) < sparsity) {
                matrix[i][j] = 0.0f;
            } else {
                matrix[i][j] = (float)(rand() % 100 + 1);
            }
        }
    }
}

float** multiply_matrices(uint64_t rowsA, uint64_t colsA, float **matrixA, uint64_t rowsB, uint64_t colsB, float **matrixB) {
    if (colsA != rowsB) {
        fprintf(stderr, "matrix multiplication not possible: incompatible dimensions.\n");
        exit(EXIT_FAILURE);
    }
    float **result = allocate_matrix(rowsA, colsB);
    for (uint64_t i = 0; i < rowsA; i++) {
        for (uint64_t j = 0; j < colsB; j++) {
            result[i][j] = 0.0f;
            for (uint64_t k = 0; k < colsA; k++) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
    return result;
}

void dense_to_ellpack(uint64_t rows, uint64_t cols, float **matrix, uint64_t max_values_per_col, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%lu,%lu,%lu\n", rows, cols, max_values_per_col);

    for (uint64_t j = 0; j < cols; j++) {
        uint64_t values_written = 0;
        for (uint64_t i = 0; i < rows; i++) {
            if (matrix[i][j] != 0 && values_written < max_values_per_col) {
                fprintf(file, "%e", matrix[i][j]);
                if (j < cols - 1 || values_written < max_values_per_col - 1) {
                    fprintf(file, ",");
                }
                values_written++;
            }
        }
        for (uint64_t k = values_written; k < max_values_per_col; k++) {
            fprintf(file, "*");
            if (j < cols - 1 || k < max_values_per_col - 1) {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    for (uint64_t j = 0; j < cols; j++) {
        uint64_t indices_written = 0;
        for (uint64_t i = 0; i < rows; i++) {
            if (matrix[i][j] != 0 && indices_written < max_values_per_col) {
                fprintf(file, "%lu", i);
                if (j < cols - 1 || indices_written < max_values_per_col - 1) {
                    fprintf(file, ",");
                }
                indices_written++;
            }
        }
        for (uint64_t k = indices_written; k < max_values_per_col; k++) {
            fprintf(file, "*");
            if (j < cols - 1 || k < max_values_per_col - 1) {
                fprintf(file, ",");
            }
        }
    }

    fclose(file);
}

void create_zero_matrix(uint64_t rows, uint64_t cols, float **matrix) {
    for (uint64_t i = 0; i < rows; i++) {
        for (uint64_t j = 0; j < cols; j++) {
            matrix[i][j] = 0.0f;
        }
    }
}

uint64_t find_max_nonzeros_per_col(float **matrix, uint64_t rows, uint64_t cols) {
    uint64_t max_nonzeros = 0;
    for (uint64_t j = 0; j < cols; j++) {
        uint64_t nonzeros = 0;
        for (uint64_t i = 0; i < rows; i++) {
            if (matrix[i][j] != 0.0f) {
                nonzeros++;
            }
        }
        if (nonzeros > max_nonzeros) {
            max_nonzeros = nonzeros;
        }
    }
    return max_nonzeros;
}

typedef struct {
    uint64_t rows;
    uint64_t cols;
    float sparsity;
} TestCase;

void run_test_case(TestCase *test_case, int counter) {
    // define the file paths
    char matrix1_filename[MAX_FILENAME_LENGTH];
    char matrix2_filename[MAX_FILENAME_LENGTH];
    char result_filename[MAX_FILENAME_LENGTH];

    snprintf(matrix1_filename, MAX_FILENAME_LENGTH, "%s%da.txt", BASE_PATH, counter);
    snprintf(matrix2_filename, MAX_FILENAME_LENGTH, "%s%db.txt", BASE_PATH, counter);
    snprintf(result_filename, MAX_FILENAME_LENGTH, "%sexpect%d.txt", BASE_PATH, counter);

    // generate the matrices
    float **matrixA = allocate_matrix(test_case->rows, test_case->cols);
    float **matrixB = allocate_matrix(test_case->cols, test_case->rows);  // ensure the second matrix can be multiplied with the first
    generate_sparse_matrix(test_case->rows, test_case->cols, matrixA, test_case->sparsity);
    generate_sparse_matrix(test_case->cols, test_case->rows, matrixB, test_case->sparsity);

    // find max non-zeros per column
    uint64_t max_values_per_colA = find_max_nonzeros_per_col(matrixA, test_case->rows, test_case->cols);
    uint64_t max_values_per_colB = find_max_nonzeros_per_col(matrixB, test_case->cols, test_case->rows);

    dense_to_ellpack(test_case->rows, test_case->cols, matrixA, max_values_per_colA, matrix1_filename);
    dense_to_ellpack(test_case->cols, test_case->rows, matrixB, max_values_per_colB, matrix2_filename);

    // verify that files are created successfully
    if (access(matrix1_filename, F_OK) != 0 || access(matrix2_filename, F_OK) != 0) {
        fprintf(stderr, "error: matrix files not created correctly\n");
        exit(EXIT_FAILURE);
    }

    float **result = multiply_matrices(test_case->rows, test_case->cols, matrixA, test_case->cols, test_case->rows, matrixB);

    uint64_t max_values_per_col_result = find_max_nonzeros_per_col(result, test_case->rows, test_case->rows);

    // save the result matrix in ellpack format
    dense_to_ellpack(test_case->rows, test_case->rows, result, max_values_per_col_result, result_filename);

    // deallocate matrices
    deallocate_matrix(matrixA, test_case->rows);
    deallocate_matrix(matrixB, test_case->cols);
    deallocate_matrix(result, test_case->rows);
}
int main() {
    TestCase test_cases[] = {
        {4, 3, 0.75},
        {5, 4, 0.80},
        {6, 5, 0.85},
        {2, 4, 0.90},
        {209, 200, 0.2},
        {100, 100, 0.5}//will have all zeros
    };

    int num_test_cases = sizeof(test_cases) / sizeof(TestCase);

    printf("Generating test cases...\n");

    // run each test case
    for (int i = 0; i < num_test_cases; i++) {
        run_test_case(&test_cases[i], i + 1);
    }

    printf("Testcases generated successfully\n");

    return EXIT_SUCCESS;
}
