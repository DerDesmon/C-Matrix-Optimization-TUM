#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "test.h"
#include "../src/matmul.h"
#include "../src/matmul_caller.h"

#define BASE_PATH "dev/inputs/dist/"

typedef enum {
    BASIC,
    OPTIMIZED
} matmul_type;

void read_file(const char *filename, ellpack_matrix *matrix) {
    // read file code as before
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "%"PRIu64",%"PRIu64",%"PRIu64"\n", &matrix->rows, &matrix->cols, &matrix->max_values_per_col) != 3) {
        fprintf(stderr, "Error reading matrix dimensions from file: %s\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    matrix->indices = (uint64_t **) malloc(matrix->cols * sizeof(uint64_t *));
    matrix->values = (float **) malloc(matrix->cols * sizeof(float *));
    for (uint64_t i = 0; i < matrix->cols; i++) {
        matrix->indices[i] = (uint64_t *) malloc(matrix->max_values_per_col * sizeof(uint64_t));
        matrix->values[i] = (float *) malloc(matrix->max_values_per_col * sizeof(float));
    }

    for (uint64_t i = 0; i < matrix->cols; i++) {
        uint64_t values_written = 0;
        for (uint64_t j = 0; j < matrix->max_values_per_col; j++) {
            char value[32];
            if (fscanf(file, "%31[^,\n]", value) == 1) {
                if (strcmp(value, "*") == 0) {
                    matrix->values[i][values_written] = 0.0f;
                } else {
                    matrix->values[i][values_written] = strtof(value, NULL);
                }
                values_written++;
            }
            fgetc(file); // skip comma
        }
        for (uint64_t k = values_written; k < matrix->max_values_per_col; k++) {
            matrix->values[i][k] = 0.0f;
        }
    }

    for (uint64_t i = 0; i < matrix->cols; i++) {
        uint64_t indices_written = 0;
        for (uint64_t j = 0; j < matrix->max_values_per_col; j++) {
            char index[32];
            if (fscanf(file, "%31[^,\n]", index) == 1) {
                if (strcmp(index, "*") == 0) {
                    matrix->indices[i][indices_written] = UINT64_MAX;
                } else {
                    matrix->indices[i][indices_written] = strtoull(index, NULL, 10);
                }
                indices_written++;
            }
            fgetc(file); // skip comma
        }
        for (uint64_t k = indices_written; k < matrix->max_values_per_col; k++) {
            matrix->indices[i][k] = UINT64_MAX;
        }
    }

    fclose(file);
}

void free_ellpack_matrix(ellpack_matrix *matrix) {
    if (matrix == NULL) return;
    if (matrix->indices != NULL) {
        for (uint64_t i = 0; i < matrix->cols; i++) {
            free(matrix->indices[i]);
        }
        free(matrix->indices);
    }
    if (matrix->values != NULL) {
        for (uint64_t i = 0; i < matrix->cols; i++) {
            free(matrix->values[i]);
        }
        free(matrix->values);
    }
}

int compare_ellpack_matrices(const ellpack_matrix *a, const ellpack_matrix *b) {
    if (a->rows != b->rows || a->cols != b->cols || a->max_values_per_col != b->max_values_per_col) {
        return 0;
    }
    for (uint64_t i = 0; i < a->cols; i++) {
        for (uint64_t j = 0; j < a->max_values_per_col; j++) {
            if (a->indices[i][j] != b->indices[i][j] || a->values[i][j] != b->values[i][j]) {
                printf("difference found at column %"PRIu64", position %"PRIu64"\n", i, j);
                printf("matrix a: value = %.2f, index = %"PRIu64"\n", a->values[i][j], a->indices[i][j]);
                printf("matrix b: value = %.2f, index = %"PRIu64"\n", b->values[i][j], b->indices[i][j]);
                return 0;
            }
        }
    }
    return 1;
}

int process_matrices(const char *matrix1_filename, const char *matrix2_filename,
                     const char *expected_result_filename, matmul_func matmul) {
    // starts here
    ellpack_matrix expected_result, matmul_result_for_comparison;

    char result_filename[256];
    snprintf(result_filename, sizeof(result_filename), "%scalc_res_%s_%s.txt", BASE_PATH, matrix1_filename,
             matrix2_filename);

    char matrix1_fullpath[256];
    char matrix2_fullpath[256];
    char expected_result_fullpath[256];

    snprintf(matrix1_fullpath, sizeof(matrix1_fullpath), "%s%s", BASE_PATH, matrix1_filename);
    snprintf(matrix2_fullpath, sizeof(matrix2_fullpath), "%s%s", BASE_PATH, matrix2_filename);
    snprintf(expected_result_fullpath, sizeof(expected_result_fullpath), "%s%s", BASE_PATH, expected_result_filename);


    if (call_matmul(matrix1_fullpath, matrix2_fullpath, result_filename, 0, matmul) ==
        EXIT_FAILURE) {
        fprintf(stderr, "Failure while calling matmul\n");
        return EXIT_FAILURE;
    }

    // read the expected result matrix
    read_file(expected_result_fullpath, &expected_result);
    read_file(result_filename, &matmul_result_for_comparison);

    // call compare function from this class on result and expected result and print results to terminal
    if (compare_ellpack_matrices(&matmul_result_for_comparison, &expected_result)) {
        printf("the multiplication result matches the expected result.\n");
    } else {
        printf("the multiplication result does not match the expected result.\n");
    }

    // free the allocated memory
    free_ellpack_matrix(&expected_result);
    free_ellpack_matrix(&matmul_result_for_comparison);
    return EXIT_SUCCESS;
}

int test_main(matmul_func matmul) {
    struct {
        const char *matrix1_filename;
        const char *matrix2_filename;
        const char *expected_result_filename;
        matmul_type type;
    } test_cases[] = {
                {"1a.txt", "1b.txt", "expect1.txt", BASIC},
                {"2a.txt", "2b.txt", "expect2.txt", BASIC},
                {"3a.txt", "3b.txt", "expect3.txt", BASIC},
                {"4a.txt", "4b.txt", "expect4.txt", BASIC},
                {"5a.txt", "5b.txt", "expect5.txt", BASIC},
                {"6a.txt", "6b.txt", "expect6.txt", BASIC},
            };

    size_t num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    // run test cases
    for (size_t i = 0; i < num_test_cases; i++) {
        printf("running test case %zu:\n", i + 1);
        process_matrices(test_cases[i].matrix1_filename, test_cases[i].matrix2_filename,
                         test_cases[i].expected_result_filename, matmul);
    }

    return EXIT_SUCCESS;
}
